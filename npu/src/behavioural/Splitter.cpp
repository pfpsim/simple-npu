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

#include "./Splitter.h"
#include <string>
#include <fstream>
#include "common/RoutingPacket.h"
#include "common/routingdefs.h"

Splitter::Splitter(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):SplitterSIM(nm,parent,configfile),outlog(OUTPUTDIR+"IngressTrace.csv") {  // NOLINT
  /*sc_spawn threads*/
  ThreadHandles.push_back(sc_spawn(sc_bind(&Splitter::SplitterThread, this, 0))); // NOLINT
}

void Splitter::init() {
  init_SIM(); /* Calls the init of sub PE's and CE's */
  add_counter("PKT_CNT");
  int isolation_groups = GetParameter("isolation_groups").get();
  for (std::size_t ig = 0; ig < isolation_groups; ig++) {
    add_counter("IG" + std::to_string(ig) + "_PKT_CNT");
  }
}

void Splitter::SplitterThread(std::size_t thread_id) {
  int counter1 = 0;
  int counter2 = 0;

  bool cpagentflag = false;

  while (1) {
    if (cpagentflag == true) {
      npulog(profile, cout <<"Starting to accept from ingress" <<endl;)
      npulog(minimal,
      cout << On_ICyan << "Starting to accept from ingress" << txtrst << endl;)

      while (1) {
        // 1. Read data from stimulus via ingress
        auto received_pkt = ingress->get();
        auto recvpkt_obj = std::dynamic_pointer_cast
                                <InputStimulus>(received_pkt);
        auto received = recvpkt_obj->stimulus_object();
        auto p = std::get<0>(received);

        npulog(debug,
        cout << dec <<"*&- Splitter pktin Counter1: "<< counter1
             << " FMG allocation Counter2: "<< counter2<< endl;)


        /* Profiling */
        npulog(profile,
          cout << "received Packet" << IBlue << p->id() << txtrst
               << " from ingress" << endl;)
        npulog(minimal,
          cout << "received Packet" << IBlue << p->id() << txtrst
               << " from ingress" << endl;)
        outlog<<p->id()<<","<<sc_time_stamp().to_default_time_units()<<endl;  // NOLINT
        counter1++;

        // 1.1. Extract data
        auto received_packet = std::get<0>(received);
        auto received_context = std::get<1>(received);
        auto received_isolation_group = std::get<2>(received);

        increment_counter
        ("IG" + std::to_string(received_isolation_group) + "_PKT_CNT");

        /* TODO:p4
        // calculate the actual length of the header part of the payload
        phv_data_t * phv;
        size_t packet_length = received_packet.data().size();
        p4_do_parsing(received_packet.data().data(), packet_length, &phv);
        size_t header_length = packet_length - phv->payload_len;
        // TODO delete that dummy PHV but there's memory leaks everywhere anyways...
         */

        // 2. Split packet payload and header
        auto start_of_header = received_packet->data().begin();
        // start_of_header + header_length;
        auto end_of_header   = received_packet->data().end();
        // 2.1 Extract the header part of the packet into a new buffer
        PacketDescriptor::raw_header_t raw_header;
        std::copy
        (start_of_header, end_of_header, std::back_inserter(raw_header));
        // 2.2 remove header from packet, so we store only the payload
        received_packet->data().erase(start_of_header, end_of_header);
        // 2.3 Create PacketDescriptor
        auto pd = std::make_shared<PacketDescriptor>(
            received_packet->id(),
            received_context,
            received_isolation_group,
            raw_header);

        // 2.4 set TTL for each packet
        // pd->TTL(4);

        // 2.4.1 Get vaddr from FMG
        auto memmessage = std::make_shared<IPC_MEM>();
        memmessage->id(received_packet->id());
        memmessage->RequestType = "ALLOCATE";
        memmessage->Allocation  = "Payload";
        memmessage->bytes_to_allocate = static_cast<int>
                                        (received_packet->size());
        // 2.4.2 Send req to FMG
        auto to_send = make_routing_packet
                       (module_name(), PathtoMemoryManager , memmessage);
        ocn_wr_if->put(to_send);

        auto fmgreply = unbox_routing_packet<IPC_MEM>(ocn_rd_if->get());
        auto ipcpkt = fmgreply->payload;
        // PacketDescriptor
        pd->payload_paddr = ipcpkt->tlm_address;
        pd->payload_target_mem_ = ipcpkt->target_mem_mod;  // routing path
        pd->payload_target_memname_ = ipcpkt->target_mem_name;  // memname
        // payload
        // routing path
        received_packet->target_mem_dest = ipcpkt->target_mem_mod;
        // memname
        received_packet->dest_target_memname_ = ipcpkt->target_mem_name;
        received_packet->payload_paddr = ipcpkt->tlm_address;
        counter2++;
        // 3. Write Payload to IDA
        auto sendtomemory = make_routing_packet
                           (module_name(),                    // Splitter
                           received_packet->target_mem_dest,  // Target Memory
                           received_packet);              // The Packet itself
        ocn_wr_if->put(sendtomemory);
        wait(1, SC_NS);
        // 4. Write PacketDescriptor to Parser
        auto sendtoparser = make_routing_packet(module_name(), "parser", pd);
        ocn_wr_if->put(sendtoparser);
      }
    } else {
      auto receivedpkt = ocn_rd_if->get();
      cpagentflag = true;
    }
  }
}
