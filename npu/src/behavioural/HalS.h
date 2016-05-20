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

#ifndef BEHAVIOURAL_HALS_H_
#define BEHAVIOURAL_HALS_H_

#include "systemc.h"  // NOLINT
#include "tlm.h"  // NOLINT
using tlm::tlm_tag;
#include "common/Packet.h"
#include "common/PacketDescriptor.h"
#include "common/IPC_MEM.h"

class HalS : public sc_interface {
 public:
  /* User Logic - Virtual Functions for interface go here */
  /**
   * Get Job from the JobQueue <Packet Descriptor>
   * @param  thread_id app layer calle thread id
   * @param  pd        packet Descriptor
   * @param  payload   payload
   * @return           true if successful
   */
  virtual bool GetJobfromSchedular(std::size_t thread_id,
                                   std::shared_ptr<PacketDescriptor>* pd,
                                   std::shared_ptr<Packet>* payload) = 0;
  /**
   * Processing to ber performed in HAL
   * @param  thread_id app layer calle thread id
   * @param  pd        PacketDescriptor to process
   * @param  payload   Payload of PacketDescriptor
   * @return           true if successful
   */
  virtual bool do_processing(std::size_t thread_id,
                             std::shared_ptr<PacketDescriptor> pd,
                             std::shared_ptr<Packet> payload) = 0;
  /**
   * Finished processing - Write back to mem and update rest of pipelines.
   * @param  thread_id  Applayer calle thread id
   * @param  pd         PacketDescriptor it processed
   * @param  payload    Payload of PacketDescriptor
   * @return            true if successful
   */
  virtual bool SendtoODE(std::size_t thread_id,
                         std::shared_ptr<PacketDescriptor> pd,
                         std::shared_ptr<Packet> payload) = 0;

  typedef std::size_t TlmType;
  /**
   * Write function called by TlmVar to write to mem.
   * @param VirtualAddress VirtualAddress to write to
   * @param data
   * @param size
   */
  virtual void tlmwrite(int VirtualAddress, int data, std::size_t size) = 0;
  /**
   * Read function to read from Memory called by TlmVar.
   * @param  VirtualAddress VirtualAddress
   * @param  data           object
   * @param  size           size of object
   * @param  val_compare    value to compare against read value
   * @return                value @ addr
   */
  virtual TlmType tlmread(int VirtualAddress, int data, std::size_t size,
        std::size_t val_compare = 0) = 0;
};
#endif  // BEHAVIOURAL_HALS_H_
