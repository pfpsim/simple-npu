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

#ifndef BEHAVIOURAL_COMMON_MEMORYUTILITY_H_
#define BEHAVIOURAL_COMMON_MEMORYUTILITY_H_
#include <string>
#include <vector>
#include "pfpsim/pfpsim.h"

typedef uint64_t tlm_addr_;

struct memdecode{
  tlm_addr_ physcialaddr;
  std::string memname;
  std::string mempath;
  bool mappingdecode;
};

class MemoryUtility {
 public:
  explicit MemoryUtility(std::string filename);
  virtual ~MemoryUtility() = default;

  json AddressMapping;

  uint64_t getMemorySize(std::string name);
  std::string getDestinationMemory(std::string CoreName);
  json AllocationTable(std::string TypeofAllocation);
  uint64_t CalculateMaxVirtualAddress(json ListofLayout);
  std::vector<std::string> GetGlobalMemoryLayout();
  memdecode decodevirtual(uint64_t vaddr);
  bool MappingDecode(std::string name);
  std::string Mapping_Key(std::string name);
  tlm_addr_ MaxVAddr;

  std::vector<tlm_addr_>memboundries;
  std::vector<std::string>memnames;
  std::vector<std::string>mempaths;
};



#endif  // BEHAVIOURAL_COMMON_MEMORYUTILITY_H_
