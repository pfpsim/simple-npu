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
 * Packet.cpp
 *
 *  Created on: Jul 17, 2014
 *      Author: kamil
 */

#include "Packet.h"
#include <string>
#include "pfpsim/core/TrType.h"

Packet::Packet()
: Packet(0, 0, 4, packet_data_type()) {
}

Packet::Packet(std::size_t id, std::size_t context, std::size_t priority,
               const packet_data_type& packet_data)
: pfp::core::TrType(id), data_(packet_data), context_(context),
priority_(priority) {
  this->type_ = "packet";
}

Packet::Packet(const PacketDescriptor& pd, const packet_data_type& packet_data)
:  pfp::core::TrType(pd.id()), data_(packet_data), context_(pd.context()) {
}

std::size_t Packet::context_id() const {
  return context_;
}

Packet::packet_data_type& Packet::metadata() {
  return metadata_;
}

void Packet::metadata(const packet_data_type& packet_data) {
  metadata_ = packet_data;
}

Packet::packet_data_type& Packet::data() {
  return data_;
}

void Packet::data(const packet_data_type& packet_data) {
  data_ = packet_data;
}

void Packet::packet_priority(std::size_t priority) {
  priority_ = priority;
}

std::size_t Packet::packet_priority() const {
  return priority_;
}

std::string Packet::data_type() const {
  return "Packet";
}
std::size_t Packet::size() const {
  return data_.size();
}
