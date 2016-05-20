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

#include "./ControlPlaneAgentHAL.h"
#include <string>
#include <vector>
#include "common/RoutingPacket.h"
#include "common/routingdefs.h"
#include "common/IPC_MEM.h"
ControlPlaneAgentHAL::ControlPlaneAgentHAL(
  sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):
            ControlPlaneAgentHALSIM(nm, parent, configfile),
            meminfo(CONFIGROOT+"MemoryAddressMapping.cfg") {
  /*sc_spawn threads*/
}

void ControlPlaneAgentHAL::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
/*
======== Functions that TLM VAR calls ==============
 */
ControlPlaneAgentHAL::TlmType
ControlPlaneAgentHAL::tlmallocate(int BytestoAllocate) {
  auto memmessage = std::make_shared<IPC_MEM>();
  memmessage->id(3146);
  memmessage->RequestType = "ALLOCATE";
  memmessage->Allocation  = "TRIE";
  memmessage->bytes_to_allocate = BytestoAllocate;
  // TODO(gordon/Lemniscate) SET function is in ControlPlaneAgent
  memmessage->table_name = "TODO";

  ocn_wr_if->put(make_routing_packet(
                 GetParent()->module_name(), PathtoMemoryManager, memmessage));
  // wait for reply from fmg
  auto received_tr = unbox_routing_packet<IPC_MEM>(ocn_rd_if->get());
  auto ipcpkt = received_tr->payload;
  return ipcpkt->tlm_address;
}

void
ControlPlaneAgentHAL::tlmwrite(int VirtualAddress, int data, TlmType size) {
  // 1. Find Where does this write go ?
  memdecode result = meminfo.decodevirtual(VirtualAddress);
  // 2. Write to mem + shadow edmems if decode return addr lies in edram region
  if (result.mappingdecode) {
    std::string AddressMapKey = meminfo.Mapping_Key(result.memname);
    auto Mapping = meminfo.AddressMapping[AddressMapKey];
    int totalclusters = SimulationParameters["clusters"].get<int>();
    for (int clusternum = 0 ; clusternum < totalclusters; clusternum++) {
      std::string Clusterpath = cluster_prefix;
      Clusterpath = Clusterpath+"["+std::to_string(clusternum)+"]";
      for (json::iterator it = Mapping.begin(); it != Mapping.end(); ++it) {
        std::string MemoryName = it.value();
        std::string pathtomem = Clusterpath+"."+MemoryName;
        auto memmessage = std::make_shared<IPC_MEM>();
        memmessage->id(3146);
        memmessage->RequestType = "WRITE";
        memmessage->bytes_to_allocate = data;
        memmessage->tlm_address = result.physcialaddr;
        ocn_wr_if->write(make_routing_packet
                        (GetParent()->module_name(), pathtomem, memmessage));
      }
    }
  } else {
    auto memmessage = std::make_shared<IPC_MEM>();
    memmessage->id(3146);
    memmessage->RequestType = "WRITE";
    memmessage->bytes_to_allocate = data;
    memmessage->tlm_address = result.physcialaddr;
    ocn_wr_if->write(make_routing_packet
                    (GetParent()->module_name(), result.mempath, memmessage));
  }
  // 3. Return Control
  return;
}

ControlPlaneAgentHAL::TlmType
ControlPlaneAgentHAL::tlmread(int VirtualAddress) {
  memdecode result = meminfo.decodevirtual(VirtualAddress);
  std::string pathtomem = "INVALID-rdcp";
  if (result.mappingdecode) {
    std::string AddressMapKey = meminfo.Mapping_Key(result.memname);
    auto Mapping = meminfo.AddressMapping[AddressMapKey];
    std::string Clusterpath = cluster_prefix;
    Clusterpath = Clusterpath+"["+std::to_string(0)+"]";
    std::string firstmem = "INVALID-READCP";
    for (json::iterator it = Mapping.begin(); it != Mapping.end(); ++it) {
      firstmem = it.value();
      break;
    }
    pathtomem = Clusterpath + "." + firstmem;
  } else {
    pathtomem = result.mempath;
  }

  auto memmessage = std::make_shared<IPC_MEM>();
  memmessage->id(3148);
  memmessage->RequestType = "READ";
  memmessage->tlm_address = result.physcialaddr;
  ocn_wr_if->put(make_routing_packet
                (GetParent()->module_name(), pathtomem, memmessage));

  auto received_tr = unbox_routing_packet<IPC_MEM>(ocn_rd_if->get());
  auto ipcpkt = received_tr->payload;
  return ipcpkt->bytes_to_allocate;
}
