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

/*
 * IPC_MEM.cpp
 *
 *  Created on: Jul 17, 2015
 *      Author: Lemniscate Snickets
 */

#include <string>
#include "IPC_MEM.h"

IPC_MEM::IPC_MEM():
target_mem_mod("Invalid Target"),
pfp::core::TrType(0),
id_(0),
bytes_to_allocate(0),
tlm_address(0),
RequestType("INVALID"),
size_(0),
tlm_data(0) {
}

IPC_MEM::IPC_MEM(std::size_t id, tlm_mem_type tlm_address,
                 std::string RequestType):
  pfp::core::TrType(id),
  tlm_address(tlm_address),
  RequestType(RequestType),
  id_(id) {
}

IPC_MEM::IPC_MEM(const IPC_MEM& pd, const packet_data_type& packet_data):
  pfp::core::TrType(pd.id_),
  data_(packet_data),
  tlm_address(pd.tlm_address),
  RequestType(pd.RequestType),
  id_(pd.id_) {
}

IPC_MEM::packet_data_type& IPC_MEM::data() {
  return data_;
}

void IPC_MEM::data(const packet_data_type& packet_data) {
  data_ = packet_data;
}

std::size_t IPC_MEM::size() const {
  return data_.size();
}

std::string IPC_MEM::data_type() const {
  return "IPC_MEM";
}


void IPC_MEM::size(std::size_t size) {
  size_ = size;
}
