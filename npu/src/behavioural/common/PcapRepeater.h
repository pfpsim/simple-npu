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

#ifndef BEHAVIOURAL_COMMON_PCAPREPEATER_H_
#define BEHAVIOURAL_COMMON_PCAPREPEATER_H_

#include <pcap/pcap.h>
#include <vector>
#include <cstdint>
#include <string>

class PcapRepeater {
 public:
  explicit PcapRepeater(std::string inputfile);
  ~PcapRepeater();

  bool hasNext();
  std::vector<uint8_t> getNext();

 private:
  pcap_t * input;
  struct pcap_pkthdr pkt_header;
  const uint8_t * pkt_data;
};

#endif  // BEHAVIOURAL_COMMON_PCAPREPEATER_H_
