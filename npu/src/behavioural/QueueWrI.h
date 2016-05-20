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

#ifndef BEHAVIOURAL_QUEUEWRI_H_
#define BEHAVIOURAL_QUEUEWRI_H_

#include "systemc.h"  // NOLINT
#include "tlm.h"  // NOLINT
using tlm::tlm_tag;

template <typename T>
class QueueWrI : public sc_interface {
 public:
  /* User Logic - Virtual Functions for interface go here */
  virtual bool nb_can_put(tlm_tag<T> * = 0) const = 0;
  virtual const sc_core::sc_event& ok_to_put(tlm_tag<T> * = 0) const = 0;
  void put(const T& t);
  virtual void write(const T&) = 0;
};

template<typename T>
void QueueWrI<T>::put(const T& t) {
    sc_process_handle this_process = sc_get_current_process_handle();
    sc_object* current_scmodule = this_process.get_parent_object();
    pfp::core::PFPObject* module;
    module = dynamic_cast<pfp::core::PFPObject*>(current_scmodule);
    module->notify_data_written(t, sc_time_stamp().to_default_time_units());
    write(t);
}
#endif  // BEHAVIOURAL_QUEUEWRI_H_
