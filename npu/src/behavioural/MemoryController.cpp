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

#include "./MemoryController.h"
#include <string>
#include "common/RoutingPacket.h"
#include "common/routingdefs.h"
// #define debug_tlm_mem_transaction 1
MemoryController::MemoryController(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):MemoryControllerSIM(nm, parent, configfile) {  // NOLINT(whitespace/line_length)
  // Search in MemoryMap for itself and get its own parameters
  sc_object* parent_ = this->get_parent_object();
  std::string parentname = parent_->basename();
  std::string modulename = parentname;
  int rdlt = GetParameter("ReadLatency").template get<int>();
  int wrlt = GetParameter("WriteLatency").template get<int>();

  sc_time rd_lat(rdlt, SC_NS);
  sc_time wr_lat(wrlt, SC_NS);
  int mem_size = GetParameter("Capacity").template get<int>();
  RD_LATENCY = rd_lat;
  WR_LATENCY = wr_lat;

  std::string memname = modulename;
  memname_ = memname;
  if (memname.find("ed") != std::string::npos) {
    std::string clustername = GetParent()->GetParent()->module_name();
    setsourcetome_ = clustername+"."+memname;
  } else {
    setsourcetome_ = memname;
  }
  /*sc_spawn threads*/
  ThreadHandles.push_back(
    sc_spawn(sc_bind(&MemoryController::MemoryControllerThread, this, 0)));
}

void MemoryController::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}

std::string MemoryController::ExtractCorefromCluster(std::string input) {
  // Extract core__x from TEC_x.core__x
  std::string s = input;
  std::string delimiter = ".";
  size_t pos = 0;
  while ((pos = s.find(delimiter)) != std::string::npos) {
    s.erase(0, pos + delimiter.length());  // token
  }
  return s;
}

void MemoryController::MemoryControllerThread(std::size_t thread_id) {
  while (1) {
    // 1. Read request
    auto received_tr = ocn_rd_if->get();
    // Handle all IPC_MEM Packets
    if (auto mem_received = try_unbox_routing_packet<IPC_MEM>(received_tr)) {
      auto ipcpkt = mem_received->payload;
      std::string ipcpkt_SentFrom = mem_received->source;
      std::string ipcpkt_SentTo = mem_received->destination;

      // Check if Read or Write operation
      if (ipcpkt->RequestType.find("WRITE") != std::string::npos) {
            tlm_write(ipcpkt->tlm_address, ipcpkt->bytes_to_allocate);
      } else if (ipcpkt->RequestType.find("READ") != std::string::npos) {
        // Perform Read operation
        ipcpkt->bytes_to_allocate = tlm_read(ipcpkt->tlm_address);

        // Time to reply the request figure out who sent it
        if (ipcpkt_SentFrom.find(core_prefix) != std::string::npos) {
          // Check if this is an On Chip or Off chip
          if (setsourcetome_.find("ed") != std::string::npos) {
            // Set extracted Local teu name from tecx.teux to destination
            ipcpkt_SentTo =  ExtractCorefromCluster(ipcpkt_SentFrom);
          } else {
            ipcpkt_SentTo = ipcpkt_SentFrom;  // set to tec.teu if not ed
          }
        } else {
          ipcpkt_SentTo = ipcpkt_SentFrom;  // Cp Agent && others
        }
        ipcpkt_SentFrom = setsourcetome_;  // set sender info to myself

        auto to_send = make_routing_packet
            (ipcpkt_SentFrom, ipcpkt_SentTo, ipcpkt);
        ocn_wr_if->put(to_send);

      } else {
        npu_error("TLMCRTLR IPC_MEM Invalid command in "+memname_);
      }
    } else if (unbox_routing_packet<>(received_tr)->source.find(core_prefix)
               != std::string::npos) {
      if (auto received_pd = try_unbox_routing_packet
                             <PacketDescriptor>(received_tr)) {
        auto pdpkt = received_pd->payload;
        std::string Requester = received_pd->source;
        if (received_pd->command.find("payload_request")
              != std::string::npos) {
          // 3. Pick data from memory
          if (memory_.find(pdpkt->id()) == memory_.end()) {
            npu_error("CDU dropped because of memory: "+memname_+" pkt id: "+std::to_string(pdpkt->id())+" Command: "+received_pd->command);  // NOLINT(whitespace/line_length)
          } else {
            auto picked = memory_.at(pdpkt->id());
            mtx_memory_.lock();
            memory_.erase(picked->id());
            mtx_memory_.unlock();
            tlm_read_cumulative(pdpkt->payload_paddr, picked->data());
            // 4. Return data to core_
            std::string PayloadFrom = setsourcetome_;
            // Figure out who to send it to based on on/off chip
            std::string PayloadTo = "INVALID-"+setsourcetome_;

            // set to core if ed
            if (setsourcetome_.find("ed") != std::string::npos) {
              PayloadTo = ExtractCorefromCluster(Requester);
            } else {
              PayloadTo = Requester;
            }

            auto to_send = make_routing_packet
                           (PayloadFrom, PayloadTo, picked);
            ocn_wr_if->put(to_send);
          }
        } else {
          npu_error("Memory Controller "+setsourcetome_ +" "+Requester+" Invalid Command "+received_pd->command);  // NOLINT(whitespace/line_length)
        }
      } else if (auto received_p =
                 try_unbox_routing_packet<Packet>(received_tr)) {
        // TODO(Lemniscate): REVISIT core_ command is storing a payload ??
        //  Does this even happen ??
        // 3. Write Packet into memory
        auto payload_ = received_p->payload;  // class Payload
        mtx_memory_.lock();
        memory_.emplace(payload_->id(), payload_);
        mtx_memory_.unlock();
        tlm_write_cumulative(payload_->payload_paddr, payload_->data());
      } else {
        npu_error
        ("MemoryController "+setsourcetome_+" core section- cast failed");
      }

    /* If block Closing Brace source == core_*/
    } else if (unbox_routing_packet<>(received_tr)->source.find("ode")
    // source == ODE
    // need to pass payload to ODE, either to drop it or move it
    //  to offchip memory (MCT)
             != std::string::npos) {
      if (auto received_pd
            = try_unbox_routing_packet<PacketDescriptor>(received_tr)) {
        auto received_p = received_pd->payload;
        std::string Requester = received_pd->source;

        if (received_pd->command.find("FetchPKT") !=std::string::npos) {
          // 2. Fetch Packet from memory
          if (memory_.find(received_p->id()) == memory_.end()) {
            npu_error("CTRLR ODE Dropped: "+memname_+" id@"+std::to_string(received_p->id())+" Command: "+received_pd->command);  // NOLINT(whitespace/line_length)
          } else {
            auto p = memory_.at(received_p->id());
            mtx_memory_.lock();
            memory_.erase(p->id());
            mtx_memory_.unlock();
            tlm_read_cumulative(received_p->payload_paddr, p->data());
            // 3. Write Packet to ODE
            auto to_send = make_routing_packet(setsourcetome_, "ode", p);
            ocn_wr_if->put(to_send);
            npulog(std::cout << "MEM" <<memname_ <<" wrote Payload" << p->id() << " to ODE" << std::endl;)  // NOLINT(whitespace/line_length)
          }
        } else {
          npu_error("MemoryController "+setsourcetome_+" ODE PD Invalid Command: "+received_pd->command)  // NOLINT(whitespace/line_length)
        }
      } else if (auto received_pd
                  = try_unbox_routing_packet<Packet>(received_tr)) {
        auto received_p = received_pd->payload;
        std::string Requester = received_pd->source;

        if (received_pd->command.find("STOREPAYLOAD") !=std::string::npos) {
          if (memory_missed.find(received_p->id()) != memory_missed.end()) {
            npu_error("ODE MCT received missed for"+std::to_string(received_p->id()));  // NOLINT(whitespace/line_length)
          } else {
            mtx_memory_.lock();
            memory_[received_p->id()] = received_p;
            mtx_memory_.unlock();
            tlm_read_cumulative
                (received_p->payload_paddr, received_p->data());
          }
        }
      } else {
        npu_error("TLMCTRLR ODE Invalid Data Structure ID:"+std::to_string(received_tr->id()));  // NOLINT(whitespace/line_length)
      }
    } else if (unbox_routing_packet<>(received_tr)->source.find("splitter")
              != std::string::npos) {
      // source ~ Splitter // store incoming payload in memory
      if (auto received_pd = try_unbox_routing_packet<Packet>(received_tr)) {
        auto received_p = received_pd->payload;
      // 2. Write data into memory; index by packet id
      mtx_memory_.lock();
      memory_.emplace(received_p->id(), received_p);
      mtx_memory_.unlock();
      tlm_write_cumulative(received_p->payload_paddr, received_p->data());
      } else {
      npu_error("Splitter Cast Error in "+setsourcetome_)
      }
    } else if (unbox_routing_packet<>(received_tr)->source.find("deparser")
               != std::string::npos) {
      //----------- DEPARSER FETCH
      if (auto received_p
              = try_unbox_routing_packet<PacketDescriptor>(received_tr)) {
        auto received_pd = received_p->payload;
        std::string Requester = received_p->source;

        // 2. Pick corresponding Packet from memory
        if (memory_.find(received_pd->id()) != memory_.end()) {
          auto picked = memory_.at(received_pd->id());
          mtx_memory_.lock();
          memory_.erase(picked->id());
          mtx_memory_.unlock();
          tlm_read_cumulative(received_pd->payload_paddr, picked->data());

          auto to_send = make_routing_packet
              (setsourcetome_, "deparser", picked);
          ocn_wr_if->put(to_send);
        } else {
          // I Dont have the payload ..... send the request back
          memory_missed.emplace(received_pd->id(), received_pd);
          std::cerr << "Memory" << setsourcetome_ << " request from Deparser - Don't have payload yet for PD: " << received_pd->id() << endl;  // NOLINT(whitespace/line_length)
          auto to_send = make_routing_packet("tm", "deparser", received_pd);
          ocn_wr_if->put(to_send);
        }
      } else {
        npu_error(setsourcetome_+" Deparser PD cast failed "+received_tr->data_type());  // NOLINT(whitespace/line_length)
      }
    } else {
      npu_error(setsourcetome_+" All casts failed - all is lost.");
    }
  }
}

void MemoryController::tlm_write_cumulative(tlm_data_type addr,
      packet_data_type& datatowrite) {
  packet_data_type data = datatowrite;
  tlm_data_type size_of_data = data.size();
  // get size of data to allocate
  double alloc_req = static_cast<float>(size_of_data) / static_cast<float>(32);
  // for(int i=0; i<ceil(alloc_req);i++){
  //   tlm_write(addr,0);
  // }
}
void MemoryController::tlm_read_cumulative(tlm_data_type addr,
      packet_data_type& datatoread) {
  packet_data_type data = datatoread;
  tlm_data_type size_of_data = data.size();
  double alloc_req = static_cast<float>(size_of_data) / static_cast<float>(32);
  // for(int i=0; i<ceil(alloc_req);i++){
  //  tlm_read(addr);
  // }
}

/*
 * ---------------------------------------------
 * RAW Byte sized Memory Operations
 * ---------------------------------------------
 */
void
MemoryController::tlm_write(tlm_data_type addr, tlm_data_type datatowrite) {
  tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
  sc_time delay = WR_LATENCY;
  // 0 for read ***** 1 for write
  tlm::tlm_command cmd = static_cast<tlm::tlm_command>(1);
  trans->set_command(cmd);
  trans->set_address(addr);
  trans->set_data_ptr(reinterpret_cast<unsigned char*>(&datatowrite));
  trans->set_data_length(4);
  trans->set_streaming_width(4);  // = data_length to indicate no streaming ?
  trans->set_byte_enable_ptr(0);  // 0 indicates unused
  trans->set_dmi_allowed(false);  // Mandatory initial value
  // Mandatory initial value
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  memory_if->b_transport(*trans, delay);

  if (trans->is_response_error()) {  // check response status and dela
    SC_REPORT_ERROR("TLM-2", "Response error from b_transport");
  }

#if debug_tlm_mem_transaction
  cout << modulename() << " transaction = { " << (cmd ? 'W' : 'R') << ", "
       << hex << addr << " } , data = " << hex << datatowrite << dec
       <<" at time " << sc_time_stamp() << " delay = " << delay << endl;
  // delay annotated onto the transport call
#endif
  wait(delay);
}

MemoryController::tlm_data_type
MemoryController::tlm_read(tlm_data_type addr) {
  std::size_t read_val = 0;
  tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
  sc_time delay = RD_LATENCY;
  // 0 for read ***** 1 for write
  tlm::tlm_command cmd = static_cast<tlm::tlm_command>(0);
  trans->set_command(cmd);
  trans->set_address(addr);
  trans->set_data_ptr(reinterpret_cast<unsigned char*>(&read_val));
  trans->set_data_length(4);
  trans->set_streaming_width(4);  // = data_length to indicate no streaming
  trans->set_byte_enable_ptr(0);  // 0 indicates unused
  trans->set_dmi_allowed(false);  // Mandatory initial value
  // Mandatory initial value
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  memory_if->b_transport(*trans, delay);
  if (trans->is_response_error()) {
    SC_REPORT_ERROR("TLM-2", "Response error from b_transport");
  }
#if debug_tlm_mem_transaction
  cout << "&&*-      transaction = { " << (cmd ? 'W' : 'R') << ", "
       << hex << addr << " } , data = " << hex << read_val << dec
       <<" at time " << sc_time_stamp()<< " delay = " << delay << endl;
#endif
  wait(delay);
  return read_val;
}
