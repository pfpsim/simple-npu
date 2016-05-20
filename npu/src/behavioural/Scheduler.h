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

#ifndef BEHAVIOURAL_SCHEDULER_H_
#define BEHAVIOURAL_SCHEDULER_H_
#include <string>
#include <vector>
#include <queue>
#include "../structural/SchedulerSIM.h"

#include "common/PacketDescriptor.h"
#include "common/RoutingPacket.h"
#include "common/routingdefs.h"
#include "common/RPCPacket.h"

class Scheduler: public SchedulerSIM {  // NOLINT(whitespace/line_length)
 public:
  SC_HAS_PROCESS(Scheduler);
  /*Constructor*/
  explicit Scheduler(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT(whitespace/line_length)
  /*Destructor*/
  virtual ~Scheduler() = default;

 public:
  void init();

 private:
  void Scheduler_PortServiceThread();
  void SchedulerThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  //! Internal buffer to store received Jobs from the global scheduler.
  std::queue<std::shared_ptr<PacketDescriptor>> JobsReceived;
  sc_event GotaJob;
  //! Internal buffer to store a Job Requests
  std::queue<std::shared_ptr<RPCMessage<SchedulerMessages>>> JobRequests;
  sc_event GotaJobRequest;
};

#endif  // BEHAVIOURAL_SCHEDULER_H_
