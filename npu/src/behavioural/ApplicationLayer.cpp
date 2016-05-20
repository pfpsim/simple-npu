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

#include "./ApplicationLayer.h"
#include <string>
#include <vector>
#include "common/ApplicationRegistry.hpp"

ApplicationLayer::ApplicationLayer(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):ApplicationLayerSIM(nm, parent, configfile) {  // NOLINT(whitespace/line_length)
    /*sc_spawn threads*/
  int teu_threads = GetParameter("core_threads").get();
  for (std::vector<sc_process_handle>::size_type i = 0; i < teu_threads; i++) {
    ThreadHandles.push_back(
      sc_spawn(sc_bind(&ApplicationLayer::ApplicationLayerThread, this, i)));
  }
}

void ApplicationLayer::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}

void ApplicationLayer::ApplicationLayerThread(std::size_t thread_id) {
  while (1) {
      std::shared_ptr<PacketDescriptor> pd;
      std::shared_ptr<Packet> payload;
      // TODO(Lemniscate): Add status checking on return from HAL
      halport->GetJobfromSchedular(thread_id, &pd, &payload);
      if (pd && payload) {
        do_processing(thread_id, std::ref(*pd.get()), std::ref(*payload.get()));
      } else {
        npulog(cout << "Something went wrong packet and/or pd were null"
                    << endl;)
      }
      halport->SendtoODE(thread_id, pd, payload);
    }
}

void ApplicationLayer::do_processing(std::size_t thread_id,
                                     PacketDescriptor& pd, Packet& payload) {
  uint32_t counter = 0;
  std::string ApplicationName = SimulationParameters["application_name"].get();
  auto received_p = call_application(ApplicationName)(counter, std::ref(pd),
      std::ref(payload), nullptr);
  payload = received_p;
  wait(counter, SC_NS);
}
