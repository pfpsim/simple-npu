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

#ifndef BEHAVIOURAL_MEMORYCONTROLLER_H_
#define BEHAVIOURAL_MEMORYCONTROLLER_H_
#include <string>
#include <vector>
#include <map>
#include "../structural/MemoryControllerSIM.h"

#include "tlm.h"  // NOLINT(build/include)
// TODO(gordon) are these still needed?
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include "CommonIncludes.h"


class MemoryController: public MemoryControllerSIM {
 public:
  SC_HAS_PROCESS(MemoryController);
  /*Constructor*/
  explicit MemoryController(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~MemoryController() = default;

 public:
  void init();

  typedef  int tlm_data_type;
  void tlm_write(tlm_data_type addr, tlm_data_type datatowrite);
  tlm_data_type tlm_read(tlm_data_type addr);
  typedef std::vector<uint8_t> packet_data_type;
  void tlm_write_cumulative(tlm_data_type addr, packet_data_type& datatowrite);
  void tlm_read_cumulative(tlm_data_type addr, packet_data_type& datatoread);

  std::string ExtractCorefromCluster(std::string);

 private:
  void MemoryControllerThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  std::string memname_;
  std::string setsourcetome_;
  sc_mutex mtx_memory_;
  //! Memory Map for Host Allocation
  std::map<std::size_t, std::shared_ptr<Packet>> memory_;
  //! Internal representation of memory
  std::map<std::size_t, std::shared_ptr<PacketDescriptor>> memory_missed;
  sc_time RD_LATENCY;
  sc_time WR_LATENCY;
};

#endif  // BEHAVIOURAL_MEMORYCONTROLLER_H_
