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

#ifndef BEHAVIOURAL_QUEUE_H_
#define BEHAVIOURAL_QUEUE_H_

#include <string>
#include "../structural/QueueSIM.h"
#include "QueueRdI.h"
#include "QueueWrI.h"

template<typename T>
class Queue:
public QueueRdI<T>,
public QueueWrI<T>,
public QueueSIM {
 public:
  /* CE Consturctor */
  Queue(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile="");  // NOLINT

  /* User Implementation of the Virtual Functions in the Interface.h file */
  // Implementing get interfaces
  virtual bool nb_can_get(tlm::tlm_tag<T> *t = 0) const {
    return fifo_.nb_can_get(t);
  }
  virtual const sc_core::sc_event &ok_to_get(tlm::tlm_tag<T> *t = 0) const {
    return fifo_.ok_to_get(t);
  }
  virtual T read(tlm::tlm_tag<T> *t = 0 ) {return fifo_.get(t);}

  // Peek interface
  virtual T peek(tlm_tag<T> *t = 0) const {
    return fifo_.peek(t);
  }

  // Implementing put interfaces
  virtual bool nb_can_put(tlm_tag<T> *t = 0) const {
    return fifo_.nb_can_put(t);}
  virtual const sc_core::sc_event& ok_to_put(tlm_tag<T> *t = 0) const  {
    return fifo_.ok_to_put(t);
  }
  virtual void write(const T& t)  {
    fifo_.put(t);
  }

 private:
    tlm::tlm_fifo<T> fifo_;
};

/*
  Queue Consturctor
 */
template<typename T>
Queue<T>::Queue
  (sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):
    QueueSIM(nm, parent, configfile),
    fifo_(static_cast<int>(GetParameter("FifoSize").get())) {
}

#endif  // BEHAVIOURAL_QUEUE_H_
