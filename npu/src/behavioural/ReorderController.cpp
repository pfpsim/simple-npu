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

#include "./ReorderController.h"
#include <deque>
#include <string>
#include "common/RoutingPacket.h"
#include "common/routingdefs.h"

#define ReorderController_REORDER_ENABLED

ReorderController::ReorderController(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):ReorderControllerSIM(nm, parent, configfile) {  // NOLINT(whitespace/line_length)
  int contexts = GetParameter("contexts").get();
  for (std::size_t context = 0; context < contexts; context++) {
    std::deque<std::size_t> po;
    std::deque<std::shared_ptr<PacketDescriptor>> pr;
    packet_order_.push_back(po);
    packet_reorder_.push_back(pr);
    idq ppo;
    pdq ppr;
    ppacket_order_.push_back(ppo);
    ppacket_reorder_.push_back(ppr);
    bool bl = false;
    pd_flags_.push_back(bl);
  }
    /*sc_spawn threads*/
#ifdef ReorderController_REORDER_ENABLED
  ThreadHandles.push_back(
      sc_spawn(sc_bind(& ReorderController::ReorderControllerThread, this, 0)));
  ThreadHandles.push_back(
      sc_spawn(
        sc_bind(
          &ReorderController::ReorderController_PortServiceThread, this)));
#else
  sc_spawn(sc_bind(& ReorderController::NOREORDER, this));
#endif
}

void ReorderController::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void ReorderController::ReorderController_PortServiceThread() {
  while (1) {
    auto received_tr = unbox_routing_packet
                       <PacketDescriptor>(ocn_rd_if->get());
    auto received_pd = received_tr->payload;

    auto context = received_pd->context();

    if (received_tr->source == cluster_scheduler_prefix) {
      packet_order_gaurd.lock();
      ppacket_order_.at(received_pd->context()).push(received_pd->id());
      packet_order_gaurd.unlock();
    } else {
      packet_reorder_gaurd.lock();
      ppacket_reorder_.at(received_pd->context()).push(received_pd);
      packet_reorder_gaurd.unlock();
      if (ppacket_reorder_.at(context).top()->id()
          == ppacket_order_.at(context).top()) {
        pd_flags_.at(context) = true;
        bufferevent.notify();
      }
    }
  }
}
void ReorderController::ReorderControllerThread(std::size_t thread_id) {
  while (1) {
    for (auto i : pd_flags_) {
      while (pd_flags_[i] == true) {
        auto context = i;
        // Pop as many as possible from packet_order and packet_reorder

        if (!ppacket_reorder_.at(context).empty() &&
            ppacket_reorder_.at(context).top()->id() ==
            ppacket_order_.at(context).top()) {
          auto to_send = make_routing_packet
                      (module_name(), "tm", ppacket_reorder_.at(context).top());
          ocn_wr_if->put(to_send);
          wait(1, SC_NS);
          ppacket_reorder_.at(context).pop();
          ppacket_order_.at(context).pop();
        } else {
          pd_flags_[i] = false;
        }
      }
    }
    wait(bufferevent);
  }
}

void ReorderController::NOREORDER() {
  while (1) {
    auto received_tr = unbox_routing_packet
                       <PacketDescriptor>(ocn_rd_if->get());
    auto received_pd = received_tr->payload;
    auto context = received_pd->context();
    if (received_tr->source.find("scheduler") != std::string::npos) {
      packet_order_gaurd.lock();
      ppacket_order_.at(received_pd->context()).push(received_pd->id());
      packet_order_gaurd.unlock();
    } else {
      ocn_wr_if->put(make_routing_packet(module_name(), "tm", received_pd));
    }
  }
}
