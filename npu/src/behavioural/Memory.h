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

#ifndef BEHAVIOURAL_MEMORY_H_
#define BEHAVIOURAL_MEMORY_H_

#include <string>
#include <vector>
#include "../structural/MemorySIM.h"
#include "MemI.h"

// #define debug_tlm_mem_transaction 1
template<typename T>
class Memory:
public MemI<T>,
public MemorySIM {
 public:
  /* CE Consturctor */
  Memory(sc_module_name nm,
         pfp::core::PFPObject* parent = 0,
         std::string configfile = "");
  /* User Implementation of the Virtual Functions in the Interface.h file */
  virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    adr = trans.get_address();
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();
    unsigned char*   byt = trans.get_byte_enable_ptr();
    unsigned int     wid = trans.get_streaming_width();
    // check address range
    if (adr >= static_cast<sc_dt::uint64>(SIZE) ||
        byt != 0 || len > 4 || wid < len) {
      std::cerr << "tlm_memory " << modulename
            << " Target does not support given mem payload transaction addr:"
            <<adr << endl;
      std::cerr << "tlm_memory - You probably have an out of range address"
            <<endl;
      SC_REPORT_ERROR("TLM-2", "Target does not support given generic payload transaction");  // NOLINT(whitespace/line_length)
      sc_stop();
    }
    // Read or Write depending upon command.
    if ( cmd == tlm::TLM_READ_COMMAND ) {
      memcpy(ptr, &mem[adr], len);
    } else if ( cmd == tlm::TLM_WRITE_COMMAND ) {
      memcpy(&mem[adr], ptr, len);
    }
    // response status to indicate successful completion
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
  }

 private:
  std::string modulename;
  std::vector<int> mem;
  uint64_t SIZE;
  sc_time RD_LATENCY;
  sc_time WR_LATENCY;
};

/*
  Memory Consturctor
 */
template<typename T>
Memory<T>::Memory
  (sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile)
  : MemorySIM(nm, parent, configfile) {
  int rdlt = GetParameter("ReadLatency").template get<int>();
  int wrlt = GetParameter("WriteLatency").template get<int>();

  sc_time rd_lat(rdlt, SC_NS);
  sc_time wr_lat(wrlt, SC_NS);
  int mem_size = GetParameter("Capacity").template get<int>();
  RD_LATENCY = rd_lat;
  WR_LATENCY = wr_lat;
  SIZE = mem_size;
  // Initialize memory with random data
  for (int i = 0; i < SIZE; i++) {
    mem.push_back(0xAA000000);
  }
}

#endif  // BEHAVIOURAL_MEMORY_H_
