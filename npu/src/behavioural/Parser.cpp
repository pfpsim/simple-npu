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

#include "./Parser.h"
#include <string>
#include <vector>
#include <chrono>

#include "common/RoutingPacket.h"

Parser::Parser(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):ParserSIM(nm, parent, configfile) {  // NOLINT(whitespace/line_length)
  /*sc_spawn threads*/
  int isolation_groups = GetParameter("isolation_groups").get();
  for (std::size_t rows = 0; rows < isolation_groups; rows++) {
    std::vector<std::size_t> new_row;
    credit_matrix_.push_back(new_row);
  }
  int rcos = GetParameter("rcos").get();
  int credit = GetParameter("aam_credit").get();
  for (std::size_t row = 0; row < credit_matrix_.size(); row++) {
    for (std::size_t col = 0; col < rcos; col++) {
      credit_matrix_[row].push_back(credit);
    }
  }
  ThreadHandles.push_back(sc_spawn(sc_bind(&Parser::ParserThread, this, 0)));
}

void Parser::init() {
  init_SIM(); /* Calls the init of sub PE's and CE's */
  add_counter("PCL_PKT_FROM_SPLITTER_ANY_EVENT");
  int isolation_groups = GetParameter("isolation_groups").get();
  for (std::size_t ig = 0; ig < isolation_groups; ig++) {
    add_counter("PCL_PKT_FROM_SPLITTER_IG" + std::to_string(ig) + "_EVENT");
    add_counter("PCL_PKT_TO_SCHEDULER_IG" + std::to_string(ig) + "_EVENT");
  }
}

void Parser::ParserThread(std::size_t thread_id) {
  while (1) {
    auto received_rp = unbox_routing_packet
                       <PacketDescriptor>(ocn_rd_if->get());
    auto received_pd = received_rp->payload;
    // 1. Who sent it ?
    if (received_rp->source == "splitter") {
      increment_counter("PCL_PKT_FROM_SPLITTER_ANY_EVENT");
      increment_counter("PCL_PKT_FROM_SPLITTER_IG" +
                        std::to_string(received_pd->isolation_group()) +
                        "_EVENT");

      // 2. Check if it can write to the port or not.
      if (!ocn_wr_if->nb_can_put()) {
        // 2.1 Drop the PD.
        if (SimulationParameters["drop"].get<bool>()) {
          drop_data(received_pd, "Parser Can't write, output fifo is full");
        } else {
          // 2.2 Wait till space is avaliable in FIFO
          wait(ocn_wr_if->ok_to_put());
        }
      }
      // 3.0 Lets parse.
      auto seed = static_cast<unsigned int>
      (std::chrono::high_resolution_clock::now().time_since_epoch().count());
      std::mt19937 rng(seed);
      auto rcos = SimulationParameters["rcos"].get<int>();
      auto priorities = SimulationParameters["priorities"].get<int>();
      std::uniform_int_distribution<std::size_t> uid_rcos(0, rcos - 1);
      std::uniform_int_distribution<std::size_t> uid_pri(0, priorities - 1);

      // 3.0.1 Parse the packet
      received_pd->parse();
      // 3.1 Choose priority for received PacketDescriptor
      //      When a packet is enqueued, it calculates the priority number.
      //      i.e. a number between 1 and 5.
      auto pri = received_pd->context() % priorities;
      // 3.2 Assigning Resource Class of Service to PacketDescriptor
      received_pd->resource_class_of_service(uid_rcos(rng));
      // 3.3 Assign prioirty to PacketDescriptor
      // Each packet enqueued to the queuing discipline is assigned a priority.
      received_pd->packet_priority(pri);
      // 4. Check if Packet needs to be dropped
      if (received_pd->drop()) {
        // 4.1 Drop it.
        drop_data(received_pd,
                  "Parser Packet marked for drop "
                  +std::to_string(received_pd->id()));
      } else {
        // 4.2. Send it to Scheduler
        ocn_wr_if->put(make_routing_packet
                      (module_name_, "scheduler", received_pd));
        increment_counter("PCL_PKT_TO_SCHEDULER_IG" + std::to_string
                              (received_pd->isolation_group()) + "_EVENT");
      }
    } else {
      npu_error(module_name()
                +" Cant Handle a PacketDescriptor from "+received_rp->source);
    }
  }
}
