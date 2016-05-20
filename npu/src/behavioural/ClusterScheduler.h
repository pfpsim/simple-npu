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

#ifndef BEHAVIOURAL_CLUSTERSCHEDULER_H_
#define BEHAVIOURAL_CLUSTERSCHEDULER_H_
#include <string>
#include <vector>
#include <queue>
#include <map>
#include "../structural/ClusterSchedulerSIM.h"
#include "common/PacketDescriptor.h"
#include "common/RoutingPacket.h"
#include "common/routingdefs.h"
#include "common/RPCPacket.h"

class ClusterScheduler: public ClusterSchedulerSIM {  // NOLINT(whitespace/line_length)
 public:
  SC_HAS_PROCESS(ClusterScheduler);
  /*Constructor*/
  explicit ClusterScheduler(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT(whitespace/line_length)
  /*Destructor*/
  virtual ~ClusterScheduler() = default;

 public:
  void init();

 private:
  void ClusterScheduler_PortServiceThread();
  void ClusterSchedulerThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;
  /**
   * Function that performs the scheduling operation
   * @return Number of destination core.
   */
  int Schedule(int CurrentScheduledCore);
  /**
   * Round Robin Scheduling
   * @param  CurrentScheduledCore
   * @return Next Core number to schedule.
   */
  int RoundRobin(int CurrentScheduledCore);
  int TotalNumberofCores;
  /**
   * Function thet fetches job from the global scheduler blocks till the global
   * scheduler replies with jobs.
   * @param numberofjobstofetch [description]
   */
  void fetchJobs(int numberofjobstofetch);

  //! Internal buffer to store received Jobs from the global scheduler.
  std::queue<std::shared_ptr<PacketDescriptor>> JobsReceived;
  std::map<std::string, int> CompletedJobs;
  sc_event ReceivedaJob;
  //! Internal buffer to store a Job Requests
  std::queue<std::shared_ptr<RPCMessage<SchedulerMessages>>> JobRequests;
  sc_event JobRequestEvent;
};

#endif  // BEHAVIOURAL_CLUSTERSCHEDULER_H_
