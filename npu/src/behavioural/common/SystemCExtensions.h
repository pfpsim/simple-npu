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
 * SystemCExtensions.h
 *
 *  Created on: Jan 10, 2016
 *      Author: Lemniscate Snickets
 */

#ifndef BEHAVIOURAL_COMMON_SYSTEMCEXTENSIONS_H_
#define BEHAVIOURAL_COMMON_SYSTEMCEXTENSIONS_H_

#include "../../QueueRdI.h"
#include "../../QueueWrI.h"
/**
 * Multi-port version of TLM's nb_can_get function
 * @param input_ports  Collection of ports to check
 * @return        True if any of the ports can get
 */
template <typename T>
inline bool nb_can_gets(const sc_vector<sc_port<QueueRdI<T>>>& input_ports) {
  bool can_get = false;
  for (auto& port : input_ports) {
    can_get |= port->nb_can_get();
  }
  return can_get;
}


/**
 * Multi-port version of TLM's ok_to_get function
 * @param input_ports  Collection of ports to check
 * @return        Or-list of the ok_to_get event from each port
 */
template <typename T>
inline sc_event_or_list
multiport_data_written_event(
  const sc_vector<sc_port<QueueRdI<T>>>& input_ports) {
  sc_event_or_list or_list;
  for (auto& port : input_ports) {
    or_list |= port->ok_to_get();
  }
  return or_list;
}



#endif  // BEHAVIOURAL_COMMON_SYSTEMCEXTENSIONS_H_
