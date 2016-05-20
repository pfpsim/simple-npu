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

#include "./TrafficManager.h"
#include <string>
#include <deque>
#include "common/RoutingPacket.h"

TrafficManager::TrafficManager(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):TrafficManagerSIM(nm, parent, configfile), weights({ 5, 3, 2, 2, 2, 1, 1, 1 }) {  // NOLINT(whitespace/line_length)
  for (std::size_t i = 0; i < c; i++) {
    std::deque<std::shared_ptr<PacketDescriptor>> pp;
    packet_order_.push_back(pp);
  }
  int priorities = GetParameter("priorities").get();
  for (std::size_t priority = 0; priority < priorities; priority++) {
    std::deque<std::shared_ptr<PacketDescriptor>> pr;
    packet_order_.push_back(pr);
  }
    /*sc_spawn threads*/
  ThreadHandles.push_back(
    sc_spawn(sc_bind(&TrafficManager::TrafficManagerThread, this, 0)));

  ThreadHandles.push_back(
    sc_spawn(sc_bind(&TrafficManager::TrafficManager_PortServiceThread, this)));
}

void TrafficManager::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void TrafficManager::TrafficManager_PortServiceThread() {
  while (1) {
    auto received_tr = ocn_rd_if->get();
    auto received_pd = std::dynamic_pointer_cast<RoutingPacket<PacketDescriptor>>(received_tr);  // NOLINT
    if (!ocn_wr_if->nb_can_put()) {
      // 2.1 Drop the PD.
      if (SimulationParameters["drop"].get<bool>()) {
        drop_data
        (received_pd, "TrafficManager Can't write, output fifo is full");
      } else {
        // 2.2 Wait till space is avaliable in FIFO
        wait(ocn_wr_if->ok_to_put());
      }
    } else {
    if (!SimulationParameters["NO_PRIORITY"].get<bool>()) {
      output_set_.push(unbox_routing_packet<PacketDescriptor>
                       (received_pd)->payload);
    } else {
      auto priority = received_pd->payload->packet_priority();
      // 2. Save order in appropriate context queue
      packet_order_.at(priority).push_back(
                unbox_routing_packet<PacketDescriptor>(received_pd)->payload);
    }
      flag_buffer_empty = false;
      condition.notify();
    }
  }
}

void TrafficManager::TrafficManagerThread(std::size_t thread_id) {
  int priorities = SimulationParameters["priorities"].get();
  while (1) {
    while (flag_buffer_empty) {
      wait(condition);
    }
    /*
    1.Pop the packetDescriptor with highest priority (0: highest)
      and send to pde
    2.When a packet is dequeued it always dequeues from
      the non-empty queuing discipline with the lowest number.
    */
    if (SimulationParameters["POLICY"].get<std::string>()
        == "FIXED_PRIORITY") {
      for (auto i = 0; i < (priorities); i++) {
        if (!packet_order_.at(i).empty()) {
          auto to_send = packet_order_.at(i).front();
          packet_order_.at(i).pop_front();
          // Write slected PD to DEPARSER
          auto send = make_routing_packet(module_name(), "deparser", to_send);
          ocn_wr_if->put(send);
          break;
        }
      }
    } else if (SimulationParameters["POLICY"].get<std::string>()
        == "WEIGHTED_ROUND_ROBIN") {
      for (size_t i = 0; i < priorities; i++) {
        if (c < max_buffer) {
          for (size_t j = 0; j < weights.at(i); j++) {
            if (!packet_order_.at(i).empty() && c < max_buffer) {
              output_set_.push(packet_order_.at(i).front());
              packet_order_.at(i).pop_front();
              c++;
            } else {
              break;
            }
          }
        } else {
          break;
        }
      }
      while (c > 0) {
        auto to_send = output_set_.pop();
        auto send = make_routing_packet(module_name(), "deparser", to_send);
        ocn_wr_if->put(send);
        c--;
      }
    } else {
      auto to_send = output_set_.pop();
      auto send = make_routing_packet(module_name(), "deparser", to_send);
      ocn_wr_if->put(send);
      npulog(profile,
             cout << "wrote " << to_send->id() << "to:deparser" << endl;)
    }
    if (
      std::all_of(
        packet_order_.begin(),
        packet_order_.end(),
        [](const std::deque<std::shared_ptr<PacketDescriptor>>& packet_queue) {
          return packet_queue.empty();
        })) {
      flag_buffer_empty = true;
    }
  }
}
