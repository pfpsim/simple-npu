/*
 * simple-npu: Example NPU simulation model using the PFPSim Framework
 *
 * Copyright (C) 2016 Concordia Univ., Montreal
 *     Samar Abdi
 *     Umair Aftab
 *     Gordon Bailey
 *     Faras Dewal
 *     Shafigh Parsazad
 *     Eric Tremblay
 *
 * Copyright (C) 2016 Ericsson
 *     Bochra Boughzala
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "./HAL.h"
#include <string>
#include "common/ApplicationRegistry.hpp"
#include "common/RoutingPacket.h"
#include "common/RPCPacket.h"
#include "common/routingdefs.h"

HAL::HAL(
  sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):
      HALSIM(nm, parent, configfile),
      sem_("Semaphore", 16),
      meminfo(CONFIGROOT+"MemoryAddressMapping.cfg") {
  tlmreqcounter = 0;
  JobRequestCounter = 0;
  /*sc_spawn threads*/
  ThreadHandles.push_back(sc_spawn(sc_bind(&HAL::HAL_PortServiceThread, this)));
}

void HAL::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
    ProcessedPDs = parent_->module_name()+"_PROCESSED_PD";
    add_counter(ProcessedPDs);
    AssignedPDs = parent_->module_name()+"_RECEIVED_PD";
    add_counter(AssignedPDs);
}

void HAL::HAL_PortServiceThread() {
  while (1) {
    auto received_tr = cluster_local_switch_rd_if->get();
    npulog(debug,
      cout << "HAL [PortService] received ID:" << received_tr->id() << endl;)
    auto ipcpkt = std::dynamic_pointer_cast<IPC_MEM>(received_tr);
    if (auto ipcpkt = try_unbox_routing_packet<IPC_MEM>(received_tr)) {
      tlmvar_halmutex.lock();
      tlmvar_halreqs_buffer.emplace(ipcpkt->payload->id(), ipcpkt->payload);
      tlmvar_halmutex.unlock();
      tlmvar_halevent.notify();
    } else if (auto received_pd =
                   try_unbox_routing_packet<PacketDescriptor>(received_tr)) {
      // job assignment: Schedular->Core
      job_queue_.push(received_pd->payload);
      evt_.notify();  // Kickstart the threads
      increment_counter(AssignedPDs);
    } else if (auto received_p =
                    try_unbox_routing_packet<Packet>(received_tr)) {
      // source == Memory requested payload
      mtx_teu_request_mem.lock();
      buffer_.emplace(received_p->payload->id(), received_p->payload);
      mtx_teu_request_mem.unlock();
      if (buffer_.find(received_p->payload->id()) != buffer_.end()) {
        payload_.notify();
      }
    } else {
      npu_error(GetParent()->module_name()+"."+module_name()+" Received Unknown Routing Packet Type")  // NOLINT(whitespace/line_length)
    }
  }
}

/*
 * ---------------------------------------------------------------------------------
 * Display Ports of Modules bound to HAL via Hal Interface
 * ---------------------------------------------------------------------------------
 */
void HAL::register_port(sc_port_base& port_, const char* if_typename_) {
  npulog(
    cout << GetParent()->module_name() << "." << module_name()
         <<"binding" << port_.name() << " to "<< "interface: "
         << if_typename_ << endl;)
}

/*
 * ---------------------------------------------------------------------------------
 * HAL Interface Utility Functions.
 * ---------------------------------------------------------------------------------
 */

bool HAL::GetJobfromSchedular(std::size_t thread_id,
                              std::shared_ptr<PacketDescriptor>* pd,
                              std::shared_ptr<Packet>* payload) {
  // this is used for the modules outside cluster when want to send back
  // to a module inside cluster. they need to know exact hierarchy
  // example: Memory
  // Traverse Hierarchy
  sc_process_handle this_process = sc_get_current_process_handle();
  sc_object* parent     = this_process.get_parent_object();  // AppLayer/HAL
  sc_object* parentCORE   = parent->get_parent_object();    // CORE
  sc_object* ParentCluster   = parentCORE->get_parent_object();  // Cluster
  const char* corename = ParentCluster->basename();
  const char* clustername = parentCORE->basename();
  name = corename;  // Cluster
  hal_core_name = std::string(clustername);
  core_number = "."+std::string(clustername);

  // 1. Try to get a Job.
  int qsize;
  job_queue_.size(qsize);
  // All threads initially wait for permission to start
  if (qsize == 0) {
    // Send out a request to the local schedular to send me a job,
    // Job buffer is empty, got to have a job.
    SchedulerMessages RequestaJob(JobRequestCounter++, "GetAJob", clustername);
    cluster_local_switch_wr_if->put(
          make_routing_packet<RPCMessage<SchedulerMessages>>(
              name + core_number,
              cluster_scheduler_prefix,
              std::make_shared<RPCMessage<SchedulerMessages>>(
                    RequestaJob.id,
                    RequestaJob)));
    wait(evt_);
  }
  // Get semaphore access
  // (number of threads that can access equal to CONFIG(teu_active_threads))
  sem_.wait();
  // TODO(?) :[Observers]
  // Get PacketDescriptor from job queue
  auto received_pd = job_queue_.pop();
  *pd = received_pd;
  // TODO(?) :[Observers]
  // Core is busy if a thread is busy

  // TODO(?) :[Observers]
  // Request the corresponding Packet from memory
  cluster_local_switch_wr_if->put(make_routing_packet(name + core_number,
        received_pd->payload_target_memname_, "payload_request", received_pd));

  mtx_payload.lock();
  payload_requested_pds.emplace(received_pd->id(), received_pd);
  wait(payload_);
  mtx_payload.unlock();
  // Check if packet exists in buffer
  bool payload_in_buffer = false;
  while (payload_in_buffer == false) {
    if (buffer_.find(received_pd->id()) == buffer_.end()) {
      wait(payload_);
    } else {
      payload_in_buffer = true;
    }
  }
  mtx_teu_request_mem.lock();
  auto received_p = buffer_.at(received_pd->id());
  mtx_teu_request_mem.unlock();
  *payload = received_p;
  return true;
}

bool HAL::do_processing(std::size_t thread_id,
                        std::shared_ptr<PacketDescriptor> pd,
                        std::shared_ptr<Packet> payload) {
  return true;
}

bool HAL::SendtoODE(std::size_t thread_id,
                    std::shared_ptr<PacketDescriptor> pd,
                    std::shared_ptr<Packet> payload) {
  // Wrap up Execution
  auto received_p = payload;
  auto received_pd = pd;

  //  1.  Send to ODE for MEM offload
  cluster_local_switch_wr_if->put(make_routing_packet(name + core_number,
                            received_pd->payload_target_memname_,
                            "completion_notice", received_p));

  // 2. Write PacketDescriptor to ODE
  // cluster_local_switch_wr_if->put(make_routing_packet(name + core_number,
  //                           "ode", received_pd));
  cluster_local_switch_wr_if->put(make_routing_packet
                                  (module_name_, "roc", received_pd));
  cluster_local_switch_wr_if->put(make_routing_packet
                              (core_number, "cluster_scheduler", received_pd));

  //  3  Now post -- all 4 tokens will become free
  sem_.post();
  increment_counter(ProcessedPDs);
  return true;
}
/*
 * ---------------------------------------------------
 * HAL TLM
 * ---------------------------------------------------
 */
std::size_t HAL::tlmread(TlmType VirtualAddress, TlmType data,
      std::size_t size, std::size_t val_compare) {
  // 1. Virtual Address
  TlmType vaddr = VirtualAddress;
  // 2. Get Physical Address from the Virtual Address Space
  memdecode result = meminfo.decodevirtual(vaddr);
  // 3.1 We need to find the target memory
  std::string destination_memory = "Not available";
  // 3.1.1 IF target mem is ed then set target edmem according to edram map
  if (result.mappingdecode) {
    std::string AddressMapKey = meminfo.Mapping_Key(result.memname);
    destination_memory
          = meminfo.getDestinationMemory(GetParent()->module_name());
  } else {
    destination_memory = result.mempath;
  }


  // 3.2 Prepare Packet to send to MEM
  // 3.2.1 set the destination memory
  auto memmessage = make_routing_packet
          (name + core_number, destination_memory, std::make_shared<IPC_MEM>());
  // 3.2.2 Set Packet ID
  memmessage->payload->id(tlmreqcounter++);
  int pktid = memmessage->payload->id();
  memmessage->payload->RequestType = "READ";
  memmessage->payload->tlm_address = result.physcialaddr;
  cluster_local_switch_wr_if->put(memmessage);
  wait(tlmvar_halevent);
  bool foundinmap = false;
  while (foundinmap == false) {
    if (tlmvar_halreqs_buffer.find(pktid) == tlmvar_halreqs_buffer.end()) {
      wait(tlmvar_halevent);
    } else {
      foundinmap = true;
    }
  }
  tlmvar_halmutex.lock();
  auto recv_p = tlmvar_halreqs_buffer.at(pktid);
  tlmvar_halmutex.unlock();

  if (recv_p->id() != pktid) {
    npu_error("MAP ERROR HAL"+core_number+
              " for id: "+std::to_string(recv_p->id()));
  }
  if (recv_p->bytes_to_allocate != val_compare) {
    npulog(
      cout << "HAL Val compare: @addr" << VirtualAddress
           << " -->" << recv_p->bytes_to_allocate
           << " -- " << val_compare
           << endl
           << "Req for vaddr:" << memmessage->payload->tlm_address
           << " got for:" << recv_p->tlm_address
           << " reqcounteris:" << tlmreqcounter << endl;)
  }
  return recv_p->bytes_to_allocate;
}

void HAL::tlmwrite(int VirtualAddress, int data, std::size_t size) {
  npu_error("HALT-WriteMEM HAL Not implemented CORE");
}
