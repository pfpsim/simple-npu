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

#include "./SortedLogger.h"
#include <string>
#include <vector>
#include <memory>
#include "common/Packet.h"
#include "common/PcapLogger.h"

SortedLogger::SortedLogger(sc_module_name nm, pfp::core::PFPObject* parent,
                           std::string configfile)
  : SortedLoggerSIM(nm, parent, configfile) {
    /*sc_spawn threads*/
  sc_spawn(sc_bind(&SortedLogger::SortedLogger_PortServiceThread, this));
}

void SortedLogger::init() {
  init_SIM(); /* Calls the init of sub PE's and CE's */
  logfilename = SPARG("validation-out");
}

namespace {
class PacketLessThan {
 public:
  bool operator() (const std::shared_ptr<Packet>& a,
                   const std::shared_ptr<Packet>& b) {
    return a->id() < b->id();
  }
};
}  // anonymous namespace

void SortedLogger::SortedLogger_PortServiceThread() {
  PacketLessThan packet_gt;

  size_t next_id = 0;
  while (1) {
    auto p = in->get();
    if (auto p1 = std::dynamic_pointer_cast<Packet>(p)) {
      auto it = std::lower_bound(packets.begin(), packets.end(), p1, packet_gt);
      packets.insert(it, p1);
    } else {
      std::cerr << "Logger Packet Dynamic Cast Failed" << endl;
      SC_REPORT_ERROR("PACKET LOGGER", "Logger Dynamic Cast Failed");
      sc_stop();
    }
  }
}

SortedLogger::~SortedLogger() {
  PcapLogger logger(logfilename);

  for (auto & p : packets) {
    logger.logPacket(p->data());
  }
}
