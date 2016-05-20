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
#ifndef BEHAVIOURAL_COMMON_RPCPACKET_H_
#define BEHAVIOURAL_COMMON_RPCPACKET_H_

#include <string>
#include "pfpsim/pfpsim.h"

/**
 * Generic RPCPacket that can be used to route messages between PE's
 */
template <typename T>
class RPCMessage : public pfp::core::TrType {
 public:
  RPCMessage(std::size_t id, T payload):
    pfp::core::TrType(id),
    Message(payload) {
    }
  ~RPCMessage() = default;

  std::string data_type() const override {
    return "RPCPacket";
  }
  T GetMessage() const {
    return Message;
  }
 private:
  const T Message;
};

/**
 * Data Structure that is used by
 */
struct SchedulerMessages {
  /**
   *  Id of this message if Command is "Get Job"
   *  Id of the pkt descriptor if Command id "Job Completed"
   */
  const std::size_t id;
  const std::string command;
  const std::string SendJobto;

  explicit SchedulerMessages(
  int messageid = 0,
  std::string command_ = "noop",
  std::string sendjobto = "invalid"):
    id(messageid),
    command(command_),
    SendJobto(sendjobto) {}
};
#endif  // BEHAVIOURAL_COMMON_RPCPACKET_H_
