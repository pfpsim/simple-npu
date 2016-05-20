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

#include "./ClusterScheduler.h"
#include <string>
#include <vector>
#include <queue>

#include "common/PacketDescriptor.h"
#include "common/RoutingPacket.h"
#include "common/routingdefs.h"
#include "common/RPCPacket.h"

ClusterScheduler::ClusterScheduler(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):ClusterSchedulerSIM(nm, parent, configfile) {  // NOLINT(whitespace/line_length)
  /*sc_spawn threads*/
  ThreadHandles.push_back(
    sc_spawn(
      sc_bind(&ClusterScheduler::ClusterSchedulerThread, this, 0)));
  ThreadHandles.push_back(
    sc_spawn(
      sc_bind(&ClusterScheduler::ClusterScheduler_PortServiceThread, this)));
}

void ClusterScheduler::init() {
  init_SIM(); /* Calls the init of sub PE's and CE's */
  TotalNumberofCores = GetParameter("cores").get<int>();
  for (int i = 0; i < TotalNumberofCores; i++) {
    CompletedJobs["core["+std::to_string(i)+"]"] = 0;
    add_counter
      ("Jobs_"+module_name()+"_."+core_prefix+"["+std::to_string(i)+"]");
  }
}
void ClusterScheduler::ClusterScheduler_PortServiceThread() {
  // Thread function to service input ports.
  while (1) {
    // Get the current transaction on my write port
    auto received_tr = cluster_local_switch_rd_if->get();
    // Unbox the Packet Descriptor
    if (auto received_pd
             = try_unbox_routing_packet<PacketDescriptor>(received_tr)) {
      // Check if its from the global Scheduler
      if (received_pd->source == "scheduler") {
        // Store it in its internal buffer
        JobsReceived.push(received_pd->payload);
        ReceivedaJob.notify();
      } else if (received_pd->source.find(core_prefix) != std::string::npos) {
        // Got a PD from a core, looks like its done with its job.
        CompletedJobs[received_pd->source]++;
        increment_counter("Jobs_"+module_name()+"_"+received_pd->source);

      } else {
        npu_error
        ("Cluster Scheduler - Can't handle a PD from "+received_pd->source);
      }
    } else if (auto request
             = try_unbox_routing_packet
                    <RPCMessage<SchedulerMessages>>(received_tr)) {
      if (request->payload->GetMessage().command == "GetAJob") {
        JobRequests.push(request->payload);
        JobRequestEvent.notify();
      } else {
        npu_error("Cluster Scheduler - Invalid Message");
      }
    } else {
      npu_error("ClusterScheduler - got something but not a pd ");
    }
  }
}
void ClusterScheduler::ClusterSchedulerThread(std::size_t thread_id) {
  // Thread function for module functionalty
  auto NumberOfCores = SimulationParameters["cores"].get<int>();
  //! Last Core that was assigned a Job
  int Assignto = 0;
  while (1) {
    if (!JobRequests.empty()) {
      // cout << "Size of Job Requests: "<<JobRequests.size()<<endl;
      // cout << "Size of Job Received: "<<JobsReceived.size()<<endl;
      if (JobsReceived.empty()) {
         fetchJobs(JobRequests.size());
       }
      auto request = JobRequests.front();
      JobRequests.pop();
      // Pop from the front of the queue
      auto JobtoAssign = JobsReceived.front();
      JobsReceived.pop();
      // Figure out which core to assign it to.
      // Assignto = Schedule(Assignto);
      // Write to ROC (for ordering)
      cluster_local_switch_wr_if->put(make_routing_packet
              (module_name(), "roc", JobtoAssign));
      // Send to core in my cluster
      cluster_local_switch_wr_if->put(make_routing_packet(
                  module_name(),
                  request->GetMessage().SendJobto,
                  JobtoAssign));
    } else {
      wait(ReceivedaJob | JobRequestEvent);
    }
  }
}

int ClusterScheduler::Schedule(int CurrentCore) {
  return RoundRobin(CurrentCore);
}

int ClusterScheduler::RoundRobin(int LastCoreScheduled) {
  return (LastCoreScheduled +1) % TotalNumberofCores;
}

void ClusterScheduler::fetchJobs(int numberofjobstofetch) {
  SchedulerMessages RequestaJob(
      numberofjobstofetch,
      "ClusterJobRequest",
      GetParent()->module_name()+"."+module_name());
  // Send to Global scheduler
  cluster_local_switch_wr_if->put(
    make_routing_packet<RPCMessage<SchedulerMessages>>(
      GetParent()->module_name()+"."+module_name(),  // Source
      global_scheduler,                              // Destination
      std::make_shared<RPCMessage<SchedulerMessages>>(0, RequestaJob)));
  wait(ReceivedaJob);  // Block till we get a job
}
