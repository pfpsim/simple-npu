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

#include "./MemoryManager.h"
#include <string>
#include "common/RoutingPacket.h"
#include "common/IPC_MEM.h"
MemoryManager::MemoryManager(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):  // NOLINT
  MemoryManagerSIM(nm, parent, configfile),
  outlog(OUTPUTDIR+"MemoryManagerAllocationTrace.csv"),
  meminfo(CONFIGROOT+"MemoryAddressMapping.cfg") {
    /*sc_spawn threads*/
  ThreadHandles.push_back
      (sc_spawn(sc_bind(&MemoryManager::MemoryManagerThread, this, 0)));
}

void MemoryManager::init() {
  init_SIM(); /* Calls the init of sub PE's and CE's */
  setsourcetome_ = module_name();

  auto globalMemConfig = meminfo.GetGlobalMemoryLayout();
  VirtualMemMaxSize = meminfo.CalculateMaxVirtualAddress(globalMemConfig);
  auto AllocationTable = meminfo.AddressMapping["ALLOCATE"];
  for (json::iterator it = AllocationTable.begin();
       it != AllocationTable.end();
       ++it) {
    std::string Name = it.key();
    uint64_t lowerrange = it.value().at(0).get<uint64_t>();
    uint64_t upperrange = it.value().at(1).get<uint64_t>();
    MemoryRegions[Name] = FreeMemoryTracker(lowerrange, upperrange);
  }

  for (auto it = MemoryRegions.begin() ; it != MemoryRegions.end(); it++) {
    // cout << "Name: "<<it -> first << endl;
    // it->second.print();
  }
}

void MemoryManager::MemoryManager_PortServiceThread() {
  // Thread function to service input ports.
}

void MemoryManager::MemoryManagerThread(std::size_t thread_id) {
  while (1) {
    auto received = unbox_routing_packet<IPC_MEM>(rd_if->get());
    auto ipcpkt = received->payload;
    if (ipcpkt->RequestType.find("ALLOCATE") !=std::string::npos) {
      auto VirtualAddress
           = Allocate(ipcpkt->bytes_to_allocate, ipcpkt->Allocation);

      auto virtualaddr = Allocate(ipcpkt->bytes_to_allocate,
                                  ipcpkt->Allocation);

      memdecode dest = meminfo.decodevirtual(virtualaddr);

      // TODO(Lemniscate): Condense it
      ipcpkt->table_name = "";
      ipcpkt->tlm_address = dest.physcialaddr;
      ipcpkt->target_mem_name = dest.memname;  // memname
      ipcpkt->decode_via_mapping = dest.mappingdecode;
      ipcpkt->target_mem_mod = dest.mempath;

      // For Debugger
      increment_counter
      (dest.memname + "Usage", wordsizetoBytes(ipcpkt->bytes_to_allocate));

      auto to_send = make_routing_packet
                     (setsourcetome_, received->source , ipcpkt);

      wr_if->put(to_send);

    } else if (ipcpkt->RequestType.find("DEALLOCATE") !=std::string::npos) {
      Deallocate(ipcpkt->Allocation,
                 ipcpkt->tlm_address,
                 ipcpkt->bytes_to_allocate);
    } else {
      std::cerr << "UNKNOWN MemoryManager COMMAND" << endl;
    }
  }
}

unsigned long MemoryManager::wordsizetoBytes(std::size_t wordsize) { //NOLINT
  return (unsigned long)ceil((float) wordsize / (float) 32);  // NOLINT
}


uint64_t
MemoryManager::Allocate(tlm_addr_ bytes_to_allocate,
                        std::string TypeofAllocation) {
  auto result =  MemoryRegions[TypeofAllocation].allocate(bytes_to_allocate);
  // cout<< "Allocate - "<<TypeofAllocation<<endl;
  // MemoryRegions[TypeofAllocation].print();
  return result;
}
bool MemoryManager::Deallocate(
  std::string TypeofAllocation,
  tlm_addr_ deaddr,
  tlm_addr_ bytes_to_deallocate) {
  return MemoryRegions[TypeofAllocation].deallocate
                                          (deaddr, bytes_to_deallocate);
}
