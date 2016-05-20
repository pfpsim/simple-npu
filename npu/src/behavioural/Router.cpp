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

#include "./Router.h"
#include <string>
#include <utility>
#include "NPU.h"
#include "common/RoutingPacket.h"

// #define RouterLogging

Router::Router(sc_module_name nm , int ocn_rd_ifPortSize, int ocn_wr_ifPortSize, pfp::core::PFPObject* parent, std::string configfile):RouterSIM(nm , ocn_rd_ifPortSize, ocn_wr_ifPortSize, parent, configfile) {  // NOLINT(whitespace/line_length)
  /*sc_spawn threads*/
  ThreadHandles.push_back(
      sc_spawn(sc_bind(&Router::Router_PortServiceThread, this)));
  ThreadHandles.push_back(
      sc_spawn(sc_bind(&Router::RouterThread, this, 0)));

  #ifdef RouterLogging
  outlog.open(OUTPUTDIR+module_name()+"out.csv");
  readlog.open(OUTPUTDIR+module_name()+"in.csv");
  #endif
}

void Router::init() {
  init_SIM(); /* Calls the init of sub PE's and CE's */
  if (parent_->module_name() == "npu") {
    ReadConfigfile(CONFIGROOT+"RoutingConfigNpu.cfg");
  } else if (parent_->module_name().find("cluster") != std::string::npos) {
    ReadConfigfile(CONFIGROOT+"RoutingConfigCluster.cfg");
  } else {
    npu_error(module_name()+" - Config File error");
  }
}
void Router::Router_PortServiceThread() {
  int readportnumber = 0;
  while (1) {
    if (!nb_can_gets(ocn_rd_if)) {
      wait(multiport_data_written_event(ocn_rd_if));
    } else {
      while (!ocn_rd_if.at(readportnumber)->nb_can_get()) {
        readportnumber = (++readportnumber)%ocn_rd_if.size();
      }
      if (ocn_rd_if.at(readportnumber)->nb_can_get()) {
        auto routepacket = unbox_routing_packet<>
                           (ocn_rd_if.at(readportnumber)->get());
        #ifdef RouterLogging
        LogPacket(readlog, routepacket);
        #endif
        auto dest = EvaluateHierarchicalDestination(routepacket->destination);
        int outputportnumber;
        bool StripHierarchicalDestination = false;
        std::tie(outputportnumber, StripHierarchicalDestination) =
             DetermineOutputPortNumber(dest.sendto);
        // Only Strip the parent in the Hierarchy if directly connected to me.
        if (StripHierarchicalDestination) {
          routepacket->destination = dest.setdestinationto;
        }
        wait(1, SC_NS);
        ocn_wr_if.at(outputportnumber)->put(routepacket);
        #ifdef RouterLogging
        LogPacket(outlog, routepacket);
        #endif
      }
    }
  }
}

void Router::RouterThread(std::size_t thread_id) {
}

Router::HierarchicalDestination
Router::EvaluateHierarchicalDestination(std::string input) {
  auto delimiterposition = input.find_first_of(delimiter);
  if (delimiterposition != std::string::npos) {
    return HierarchicalDestination {
      input.substr(0, delimiterposition),
      input.substr(delimiterposition+1, input.size())
    };
  } else {
    return HierarchicalDestination {input, input};
  }
}

std::pair<int, bool>
Router::DetermineOutputPortNumber(std::string destination) {
  int portnumber = 0;
  bool result = false;
  // First check if it is a directly connected Node to me
  std::tie(portnumber, result) = SearchinConnectedNodes(destination);
  if (result == false) {
    std::string NextNode = "INVALID";
    // Whelp not a connected node, check if nextnode is defined.
    std::tie(NextNode, result) = SearchinNextNodes(destination);
    if (result == false) {
      npu_error(module_name()+" - Invalid destination: "+destination);
    } else {
      // Get portnumber to write to for next node for this destination.
      std::tie(portnumber, result) = SearchinConnectedNodes(NextNode);
      if (result == false) {
        npu_error(module_name()+" - Invalid NextNode for: "+NextNode);
      } else {
        return std::make_pair(portnumber, false);
      }
    }
  } else {
    return std::make_pair(portnumber, true);
  }
}

std::pair<int, bool> Router::SearchinConnectedNodes(std::string searchnode) {
  auto ConnectedNodes  = lookuptables["ConnectedNodes"];
  int outputportnumber = 0;
  for (json::iterator it = ConnectedNodes.begin();
       it != ConnectedNodes.end(); ++it) {
    if (*it == searchnode) {
      return std::make_pair(outputportnumber, true);
    }
    outputportnumber++;
  }
  return std::make_pair(0, false);
}

std::pair<std::string, bool> Router::SearchinNextNodes(std::string searchnode) {
  auto NextNodes = lookuptables["NextNodes"];
  for (json::iterator it = NextNodes.begin(); it != NextNodes.end(); ++it) {
    if (it.key() == searchnode) {
      std::string NextNode = it.value();
      return std::make_pair(NextNode, true);
    }
  }
  return std::make_pair("INVALID", false);
}

void Router::ReadConfigfile(std::string filename) {
  std::ifstream ifs(filename);
  if (!ifs.fail()) {
    std::string content((std::istreambuf_iterator<char>(ifs) ),
                        (std::istreambuf_iterator<char>()));
    auto ConfigFile = json::parse(content);
    lookuptables = ConfigFile[module_name()];
  } else {
    std::cerr << On_Purple<< "Could not open file: "<< txtrst
              << On_Red<< filename<< txtrst<< std::endl;
    exit(-1);
  }
}

void Router::LogPacket
    (std::ofstream& outputto, std::shared_ptr<AbstractRoutingPacket> packet) {
  if (try_unbox_routing_packet<PacketDescriptor>(packet)) {
    auto packet_in = unbox_routing_packet<PacketDescriptor>(packet);
    auto routepacket = packet_in->payload;
    outputto<<sc_time_stamp().to_default_time_units()<<","<<routepacket->data_type()<<" @: ,"<<routepacket->id()<<","<<packet_in->source<<","<<packet_in->destination<<endl;  // NOLINT
  } else if (try_unbox_routing_packet<Packet>(packet)) {
    auto packet_in = unbox_routing_packet<Packet>(packet);
    auto routepacket = packet_in->payload;
    outputto<<sc_time_stamp().to_default_time_units()<<","<<routepacket->data_type()<<" @: ,"<<routepacket->id()<<","<<packet_in->source<<","<<packet_in->destination<<endl;  // NOLINT
  } else if (try_unbox_routing_packet<IPC_MEM>(packet)) {
    auto packet_in = unbox_routing_packet<IPC_MEM>(packet);
    auto routepacket = packet_in->payload;
    outputto<<sc_time_stamp().to_default_time_units()<<","<<routepacket->data_type()<<" @: ,"<<routepacket->id()<<","<<packet_in->source<<","<<packet_in->destination<<endl;  // NOLINT
  } else {
    npu_error("Whelp router has an unknown type");
  }
  return;
}
