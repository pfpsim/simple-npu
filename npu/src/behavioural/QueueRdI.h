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

#ifndef BEHAVIOURAL_QUEUERDI_H_
#define BEHAVIOURAL_QUEUERDI_H_

#include "systemc.h"  // NOLINT
#include "tlm.h"  // NOLINT
using tlm::tlm_tag;
#include "pfpsim/pfpsim.h"

template <typename T>
class QueueRdI : public sc_interface {
 public:
    /* User Logic - Virtual Functions for interface go here */
    virtual bool nb_can_get(tlm::tlm_tag<T> * = 0) const = 0;
    virtual const sc_core::sc_event &ok_to_get(tlm::tlm_tag<T> * = 0) const = 0;
    T get(tlm::tlm_tag<T> *t = 0);
    virtual T read(tlm::tlm_tag<T> * = 0) = 0;
    virtual T peek(tlm_tag<T> * = 0) const = 0;
};

template<typename T>
T QueueRdI<T>::get(tlm::tlm_tag<T> *t) {
    sc_process_handle this_process = sc_get_current_process_handle();
    sc_object* current_scmodule = this_process.get_parent_object();
    pfp::core::PFPObject* module;
    module = dynamic_cast<pfp::core::PFPObject*>(current_scmodule);
    auto ret_val =  read(t);
    module->notify_data_read(ret_val, sc_time_stamp().to_default_time_units());
    return ret_val;
}


template <typename T>
inline bool nb_can_gets(const sc_vector<sc_port<QueueRdI<T>>>& input_ports) {
    bool can_get = false;
    for (auto& port : input_ports) {
        can_get |= port->nb_can_get();
    }
    return can_get;
}

template <typename T>
inline sc_event_or_list
multiport_data_written_event
(const sc_vector<sc_port<QueueRdI<T>>>& input_ports) {
    sc_event_or_list or_list;
    for (auto& port : input_ports) {
        or_list |= port->ok_to_get();
    }
    return or_list;
}
#endif  // BEHAVIOURAL_QUEUERDI_H_
