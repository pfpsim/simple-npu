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

#include "./Logger.h"
#include <string>

Logger::Logger(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):LoggerSIM(nm, parent, configfile) {  // NOLINT(whitespace/line_length)
    /*sc_spawn threads*/
  ThreadHandles.push_back(
      sc_spawn(sc_bind(&Logger::Logger_PortServiceThread, this)));
}

void Logger::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void Logger::Logger_PortServiceThread() {
  std::string outputfile;
  if (SimulationParameters["use-validation-out"].get().get<bool>()) {
    outputfile = SPARG("validation-out");
  } else {
    outputfile = "output.pcap";
  }

  PcapLogger logger(outputfile);

  while (1) {
    auto p = in->get();
    auto p1 = std::dynamic_pointer_cast<Packet>(p);
    if (p1) {
      logger.logPacket(p1->data());
    } else {
      npu_error("Logger Packet Dynamic Cast Failed");
    }
    out->put(p);
  }
}
