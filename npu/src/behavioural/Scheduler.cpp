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

#include "./Scheduler.h"
#include <string>
#include <array>
#include <algorithm>

#include "common/PacketDescriptor.h"
#include "common/RoutingPacket.h"
#include "common/routingdefs.h"
#include "common/RPCPacket.h"

Scheduler::Scheduler(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):SchedulerSIM(nm, parent, configfile) {  // NOLINT(whitespace/line_length)
  /*sc_spawn threads*/
  ThreadHandles.push_back(
    sc_spawn(
      sc_bind(&Scheduler::Scheduler_PortServiceThread, this)));
  ThreadHandles.push_back(
    sc_spawn(
      sc_bind(&Scheduler::SchedulerThread, this, 0)));
}

void Scheduler::init() {
  init_SIM(); /* Calls the init of sub PE's and CE's */
}
void Scheduler::Scheduler_PortServiceThread() {
  // Thread function to service input ports.
  while (1) {
    auto received_tr = ocn_rd_if->get();
    if (auto received_pd
            = try_unbox_routing_packet<PacketDescriptor>(received_tr)) {
      // cout << "GOT FROM: "<< received_pd->destination<<endl;
      JobsReceived.push(received_pd->payload);
      GotaJob.notify();
    } else if (auto request
                = try_unbox_routing_packet
                  <RPCMessage<SchedulerMessages>>(received_tr)) {
      // cout << "GOT FROM: "<< request->destination<<endl;
      JobRequests.push(request->payload);
      GotaJobRequest.notify();
    } else {
      npu_error(module_name()+" Can't Handle: "+received_tr->data_type());
    }
  }
}
void Scheduler::SchedulerThread(std::size_t thread_id) {
  // Thread function for module functionalty
  while (1) {
    if (!JobRequests.empty() && !JobsReceived.empty()) {
      // Scheduling here FIFO
      auto Request = JobRequests.front();
      JobRequests.pop();
      std::size_t numberofJobsRequested = Request->GetMessage().id;
      for (int i = 0;
           i < std::min(numberofJobsRequested, JobsReceived.size());
           i++) {
        auto Job = JobsReceived.front();
        JobsReceived.pop();
        // Send the PD to the Requester.
        ocn_wr_if->put(make_routing_packet<PacketDescriptor>(
          module_name(),
          Request->GetMessage().SendJobto,
          Job));
      }
    } else {
      wait(GotaJobRequest | GotaJob);
    }
  }
}
