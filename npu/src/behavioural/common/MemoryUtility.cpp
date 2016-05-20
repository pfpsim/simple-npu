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

#include "MemoryUtility.h"
#include <string>
#include <vector>

MemoryUtility::MemoryUtility(std::string filename) {
  std::ifstream ifs(filename);
  if (!ifs.fail()) {
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
    AddressMapping = json::parse(content);
  } else {
    std::cerr << On_Purple << "Could not open file: " << txtrst << On_Red
          << filename << txtrst << std::endl;
    exit(-1);
  }
  auto globalMemConfig = GetGlobalMemoryLayout();
  MaxVAddr = CalculateMaxVirtualAddress(globalMemConfig);
}

uint64_t MemoryUtility::getMemorySize(std::string name) {
  return AddressMapping["Memories"][name]["size"].get<uint64_t>();
}

std::string MemoryUtility::getDestinationMemory(std::string CoreName) {
  return AddressMapping["CoreMapping"][CoreName].get<std::string>();
}

json MemoryUtility::AllocationTable(std::string TypeofAllocation) {
  return AddressMapping["ALLOCATE"][TypeofAllocation];
}

bool MemoryUtility::MappingDecode(std::string name) {
  return AddressMapping["Memories"][name]["Mapping"].get<bool>();
}

std::string MemoryUtility::Mapping_Key(std::string name) {
  return AddressMapping["Memories"][name]["Mapping_Key"].get<std::string>();
}

uint64_t MemoryUtility::CalculateMaxVirtualAddress(json ListofLayout) {
  uint64_t MaxVirtualAddress = 0;
  for (json::iterator it = ListofLayout.begin();
        it != ListofLayout.end();
        ++it) {
    auto MemoryName = it->get<std::string>();
    uint64_t Size = getMemorySize(MemoryName);
    MaxVirtualAddress = MaxVirtualAddress + Size;
  }
  return MaxVirtualAddress;
}
std::vector<std::string> MemoryUtility::GetGlobalMemoryLayout() {
  return AddressMapping["MemoryLayout"].get<std::vector<std::string>>();
}

memdecode
MemoryUtility::decodevirtual(tlm_addr_ virtualaddr) {
  memdecode result;
  if (virtualaddr >= 0 && virtualaddr <= MaxVAddr) {
    auto Layout = AddressMapping["MemoryLayout"];
    tlm_addr_ boundary = 0;
    for (json::iterator it = Layout.begin();
         it != Layout.end(); ++it) {
      std::string Memname = it->get<std::string>();
      if (virtualaddr < getMemorySize(Memname)) {
        result.physcialaddr = virtualaddr - boundary;
        result.memname = Memname;
        result.mappingdecode = MappingDecode(Memname);
        result.mempath = Memname;
        return result;
      } else {
        boundary = boundary + getMemorySize(Memname);
      }
    }
    npu_error("Decode Error - Can't Deocde ");
  } else {
    npu_error
    ("Virtual Address Out of range - Decode "+std::to_string(virtualaddr));
  }
}
