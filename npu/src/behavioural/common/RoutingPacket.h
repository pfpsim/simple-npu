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

#ifndef BEHAVIOURAL_COMMON_ROUTINGPACKET_H_
#define BEHAVIOURAL_COMMON_ROUTINGPACKET_H_

#include <memory>
#include <cassert>
#include <string>
#include "pfpsim/core/TrType.h"

/**
 *  This class represents an abstract packet in the NOC. It holds a source, destination
 *  and command. TODO: command should probably not be part of this class from a semantic
 *  point of view, but it is convenient.
 *
 *  This class is never instantiated directly. It is only directly used by PEs that handle
 *  routing but don't care about the content of packets.
 */
class AbstractRoutingPacket : public pfp::core::TrType {
 public:
    AbstractRoutingPacket(std::string source,
                          std::string destination,
                          std::string cmd)
      : source(source),
        destination(destination),
        command(cmd)
    {}

    virtual ~AbstractRoutingPacket() = default;

    std::string source;
    std::string destination;
    const std::string command;

    bool debuggable() const override{
      return false;
    }
};

/**
 *  This class is the concrete subclass of AbstractRoutingPacket. It holds a shared_ptr
 *  to an arbitrary type, in addition to the routing info held in the base class.
 */
template <typename T>
class RoutingPacket : public AbstractRoutingPacket {
  static_assert(std::is_base_of<pfp::core::TrType, T>::value,
                "RoutingPacket payload type must have an id getter");

 public:
    RoutingPacket(std::string source,
                  std::string destination,
                  std::shared_ptr<T> payload)
      : AbstractRoutingPacket(source, destination, "noop"),
        payload(payload)
    {}

    RoutingPacket(std::string source,
                  std::string destination,
                  std::string cmd,
                  std::shared_ptr<T> payload)
      : AbstractRoutingPacket(source, destination, cmd),
        payload(payload)
    {}

    virtual ~RoutingPacket() = default;

    const std::shared_ptr<T> payload;

    std::size_t id() const override {
      if (payload) {
        return payload->id();
      } else {
        return -1;
      }
    }

    void id(std::size_t new_id) override {
      if (payload) {
        payload->id(new_id);
      }
    }

    std::string data_type() const override {
      return "RoutingPacket";
    }

    bool debuggable() const override{
      return payload->debuggable();
    }
};

// These are wrappers to make constructing Routing packets more convenient
// since it can get pretty verbose if you have to specify the
// full templated type in a call to make_shared

template <typename T>
std::shared_ptr<RoutingPacket<T>> make_routing_packet(std::string source,
                                                      std::string destination,
                                                  std::shared_ptr<T> payload) {
  return std::make_shared<RoutingPacket<T>>(source, destination, payload);
}

template <typename T>
std::shared_ptr<RoutingPacket<T>> make_routing_packet(std::string source,
                                                      std::string destination,
                                                      std::string command,
                                                  std::shared_ptr<T> payload) {
  return std::make_shared<RoutingPacket<T>>
  (source, destination, command, payload);
}

/**
 *  This function is used to dynamic cast a TrType to RoutingPacket<T>, and assert that the
 *  cast was successful. This encapsulates a very long and verbose and often used functionality
 *  in an easy to use wrapper.
 *
 *  Additionally, if called without any template arguments, this function will cast to
 *  an AbstractRoutingPacket, allowing PEs that only care about routing to route packets
 *  regardless of their payload type.
 */
template <typename T, typename X = typename std::enable_if<!std::is_void<T>::value>::type>  // NOLINT
std::shared_ptr<RoutingPacket<T>>
unbox_routing_packet(std::shared_ptr<pfp::core::TrType> tr) {
  auto unboxed = std::dynamic_pointer_cast<RoutingPacket<T>>(tr);
  assert(unboxed);
  return unboxed;
}

template <typename T = void, typename X = typename std::enable_if<std::is_void<T>::value>::type>  // NOLINT
std::shared_ptr<AbstractRoutingPacket>
unbox_routing_packet(std::shared_ptr<pfp::core::TrType> tr) {
  auto unboxed = std::dynamic_pointer_cast<AbstractRoutingPacket>(tr);
  assert(unboxed);
  return unboxed;
}

/**
 *  This function simply does a dynamic cast from TrType to the specified RoutingPacket type. It
 *  returns the result of the cast directly without checking it. As with `unbox_routing_packet`
 *  this function can be called with an empty parameter list to cast to AbstractRoutingPacket.
 */
template <typename T, typename X = typename std::enable_if<!std::is_void<T>::value>::type>  //  NOLINT
std::shared_ptr<RoutingPacket<T>>
try_unbox_routing_packet(std::shared_ptr<pfp::core::TrType> tr) {
  return std::dynamic_pointer_cast<RoutingPacket<T>>(tr);
}

template <typename T = void, typename X = typename std::enable_if<std::is_void<T>::value>::type>  // NOLINT
std::shared_ptr<AbstractRoutingPacket>
try_unbox_routing_packet(std::shared_ptr<pfp::core::TrType> tr) {
  return std::dynamic_pointer_cast<AbstractRoutingPacket>(tr);
}

#endif  // BEHAVIOURAL_COMMON_ROUTINGPACKET_H_
