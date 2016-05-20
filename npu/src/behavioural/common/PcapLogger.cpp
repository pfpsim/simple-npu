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

#include "PcapLogger.h"
#include <sys/time.h>
#include <string>
#include <vector>
#include "pfpsim/pfpsim.h"

PcapLogger::PcapLogger(std::string outputfile) {
  pcap_t * dummy = pcap_open_dead(DLT_EN10MB, 65535);
  dumper         = pcap_dump_open(dummy, outputfile.c_str());
}

void PcapLogger::logPacket(std::vector<uint8_t> & data) {
  struct pcap_pkthdr pkt_header;
  gettimeofday(&pkt_header.ts, NULL);
  pkt_header.len = pkt_header.caplen = data.size();
  pcap_dump((u_char *)dumper, &pkt_header, data.data());  // NOLINT
}

PcapLogger::~PcapLogger() {
  pcap_dump_close(dumper);
}
