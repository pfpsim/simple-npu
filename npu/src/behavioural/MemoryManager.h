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

#ifndef BEHAVIOURAL_MEMORYMANAGER_H_
#define BEHAVIOURAL_MEMORYMANAGER_H_
#include <string>
#include <vector>
#include <map>
#include "../structural/MemoryManagerSIM.h"
#include "pfpsim/pfpsim.h"
#include "common/MemoryUtility.h"
#include "common/FreeMemoryTracker.h"

class MemoryManager: public MemoryManagerSIM {
 public:
  SC_HAS_PROCESS(MemoryManager);
  /*Constructor*/
  explicit MemoryManager(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~MemoryManager() = default;

 public:
  void init();

 private:
  void MemoryManager_PortServiceThread();
  void MemoryManagerThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  tlm_addr_ TlmAllocate(tlm_addr_ size_of_data);

  std::vector<tlm_addr_> counter;
  tlm_addr_ addrcounter = 0;
  int totalsize = 0;

  std::string setsourcetome_;
  std::string parentmod_;
  std::ofstream outlog;

  unsigned long wordsizetoBytes(std::size_t wordsize);  // NOLINT(runtime/int)

  MemoryUtility meminfo;
  uint64_t VirtualMemMaxSize;
  std::map<std::string, uint64_t> FreeAddrHeads;
  std::map<std::string, FreeMemoryTracker> MemoryRegions;
  uint64_t Allocate(tlm_addr_ bytes_to_allocate,
                    std::string TypeofAllocation);
  bool Deallocate(std::string TypeofAllocation, tlm_addr_ deaddr,
                  tlm_addr_ bytes_to_deallocate);
};

#endif  // BEHAVIOURAL_MEMORYMANAGER_H_
