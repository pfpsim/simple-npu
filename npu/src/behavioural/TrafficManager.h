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

#ifndef BEHAVIOURAL_TRAFFICMANAGER_H_
#define BEHAVIOURAL_TRAFFICMANAGER_H_
#include <string>
#include <vector>
#include <deque>

#include "../structural/TrafficManagerSIM.h"

#include "CommonIncludes.h"

class TrafficManager: public TrafficManagerSIM {
 public:
  SC_HAS_PROCESS(TrafficManager);
  /*Constructor*/
  explicit TrafficManager(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~TrafficManager() = default;

 public:
  void init();

 private:
  void TrafficManager_PortServiceThread();
  void TrafficManagerThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  //! Store the Packet order for each priority
  std::vector<std::deque<std::shared_ptr<PacketDescriptor>>> packet_order_;
  //! Deparser output set, which is filled by policer
  MTQueue<std::shared_ptr<PacketDescriptor>> output_set_;
  int c = 0;
  //! Weights for each priority, used in Weighted Round Robin policy
  std::vector<std::size_t> weights;
  //! Buffer size in Weighted Round Robin
  std::size_t max_buffer = 5;
  //! Flag for incoming packets*/
  bool flag_buffer_empty = true;
  //! To notify next thread when recieved a packet
  sc_event condition;
};

#endif  // BEHAVIOURAL_TRAFFICMANAGER_H_
