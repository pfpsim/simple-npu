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

#include "PcapRepeater.h"
#include <algorithm>
#include <vector>
#include <string>
#include "pfpsim/pfpsim.h"

PcapRepeater::PcapRepeater(std::string inputfile) {
  char errbuf[PCAP_ERRBUF_SIZE];

  input = pcap_open_offline((inputfile).c_str(), errbuf);

  if (!input) {
    std::cout << "Could not open pcap_input file "
              << inputfile
              << "(" << errbuf << ")" << std::endl;
    assert(input);
  }

  pkt_data = pcap_next(input, &pkt_header);
  }

PcapRepeater::~PcapRepeater() {
  pcap_close(input);
}

bool PcapRepeater::hasNext() {
  return !!pkt_data;
}

std::vector<uint8_t> PcapRepeater::getNext() {
  std::vector<uint8_t> vec(pkt_header.caplen);
  if (pkt_data) {
    std::copy(pkt_data, pkt_data + pkt_header.caplen, vec.begin());
    pkt_data = pcap_next(input, &pkt_header);
  }

  return vec;
}
