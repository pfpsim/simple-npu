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
 * PacketDescriptor.cpp
 *
 *  Created on: Jul 17, 2014
 *      Author: kamil
 */


#include "PacketDescriptor.h"
#include <array>
#include <algorithm>
#include <string>
#include "Packet.h"
#include "P4.h"



PacketDescriptor::PacketDescriptor() {
}


PacketDescriptor::PacketDescriptor(std::size_t id, std::size_t packet_context,
                                   std::size_t packet_isolation_group,
                                   raw_header_t raw_header,
                                   std::size_t packet_rcos,
                                   std::size_t priority)
    : pfp::core::PacketBase(id),
      packet_context_(packet_context),
      packet_ig_(packet_isolation_group),
      packet_rcos_(packet_rcos),
      packet_priority_(priority),
      packet_header_(P4::get("npu")->
        new_packet_ptr(
           0/*ingress_port*/, id,
           raw_header.size(),
           bm::PacketBuffer(
              raw_header.size(),  // capacity (TODO(gordon) be less arbitrary)
              raw_header.data(),
              raw_header.size()))) {
}

bool PacketDescriptor::operator==(const PacketDescriptor& other) const {
  return id() == other.id() &&
       packet_context_ == other.packet_context_ &&
       packet_ig_ == other.packet_ig_ &&
       packet_rcos_ == other.packet_rcos_ &&
       packet_header_ == other.packet_header_ &&
       drop() == other.drop();
}

int PacketDescriptor::parse() {
  P4::get("npu")->get_parser("parser")->parse(packet_header_.get());

  // set drop false
  drop(false);

  return 1;  // nStates;
}

void PacketDescriptor::swapPayload(uint8_t * payload, size_t length,
                          uint8_t ** old_payload, size_t * old_length) {
  // TODO(gb) do we need this? How's it gonna work with the new stuff?
  /*
  if(old_payload) *old_payload = packet_header_->payload;
  if(old_length)  *old_length  = packet_header_->payload_len;

  packet_header_->payload = payload;
  packet_header_->payload_len = length;
  */
}

PacketDescriptor::raw_header_t PacketDescriptor::deparse() {
  raw_header_t rh;
  P4::get("npu")->get_deparser("deparser")->deparse(packet_header_.get());
  char    * buf = packet_header_->data();
  size_t    len = packet_header_->get_data_size();

  std::copy(buf, buf + len, std::back_inserter(rh));

  return rh;
}

void PacketDescriptor::packet_priority(std::size_t priority) {
  packet_priority_ = priority;
}

std::size_t PacketDescriptor::packet_priority() const {
  return packet_priority_;
}

std::size_t PacketDescriptor::context() const {
  return packet_context_;
}

void PacketDescriptor::context(std::size_t packet_context) {
  packet_context_ = packet_context;
}

std::size_t PacketDescriptor::resource_class_of_service() const {
  return packet_rcos_;
}

void PacketDescriptor::resource_class_of_service(std::size_t packet_rcos) {
  packet_rcos_ = packet_rcos;
}

bool PacketDescriptor::drop() const {
  auto egress_spec = packet_header_->get_phv()->
    get_field("standard_metadata.egress_spec");
  return egress_spec.get_int() == 511;
}

void PacketDescriptor::drop(const bool &packet_drop) {
  // TODO(gordon) standardize somewhere?
  auto egress_spec = packet_header_->get_phv()->
    get_field("standard_metadata.egress_spec");

  if (packet_drop) {
    egress_spec.set(packet_drop ? 511 : 0);
  } else {
    // We only overwrite the egress spec in this case if
    // necessary, because we don't want to overwrite its
    // value if it was already not going to be dropped
    if (egress_spec.get_int() == 511) {
       egress_spec.set(0);
    }
  }
}

PacketDescriptor::header_t& PacketDescriptor::header() {
  return packet_header_;
}

void PacketDescriptor::header(const PacketDescriptor::header_t& packet_header) {
  packet_header_ = packet_header;
}

std::size_t PacketDescriptor::isolation_group() const {
  return packet_ig_;
}

void PacketDescriptor::isolation_group(std::size_t packet_isolation_group) {
  packet_ig_ = packet_isolation_group;
}

std::string PacketDescriptor::data_type() const {
  return "PacketDescriptor";
}

bool PacketDescriptor::debuggable() const {
  return true;
}
