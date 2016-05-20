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

#ifndef BEHAVIOURAL_HAL_H_
#define BEHAVIOURAL_HAL_H_
#include <string>
#include <vector>
#include <map>
#include "../structural/HALSIM.h"
#include "common/RoutingPacket.h"
#include "common/routingdefs.h"
#include "common/IPC_MEM.h"
#include "common/MemoryUtility.h"

class HAL: public HALSIM {
 public:
  SC_HAS_PROCESS(HAL);
  /*Constructor*/
  explicit HAL(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~HAL() = default;

 public:
  void init();
  typedef int TlmType;
  /*
   * HALIF Functions
   */
  virtual bool GetJobfromSchedular(std::size_t thread_id,
                                   std::shared_ptr<PacketDescriptor>* pd,
                                   std::shared_ptr<Packet>* payload);
  virtual bool do_processing(std::size_t thread_id,
                             std::shared_ptr<PacketDescriptor> pd,
                             std::shared_ptr<Packet> payload);
  virtual bool SendtoODE(std::size_t thread_id,
                         std::shared_ptr<PacketDescriptor> pd,
                         std::shared_ptr<Packet> payload);

/**
 * Write function called by TlmVar to write to mem.
 * @param VirtualAddress VirtualAddress to write to
 * @param data
 * @param size
 */
  virtual void tlmwrite(int VirtualAddress, int data, std::size_t size);
  /**
   * Read function to read from Memory called by TlmVar.
   * @param  VirtualAddress VirtualAddress
   * @param  data           object
   * @param  size           size of object
   * @param  val_compare    value to compare against read value
   * @return                value @ addr
   */
  virtual std::size_t tlmread(TlmType VirtualAddress, TlmType data,
      std::size_t size, std::size_t val_compare = 0);
  /**
   * * Prints out Port Bindings to HAL via HALInterface
   * @param port_        port
   * @param if_typename_ name
   */
  void register_port(sc_port_base& port_, const char* if_typename_);

 private:
  void HAL_PortServiceThread();
  std::vector<sc_process_handle> ThreadHandles;
  //! Event to signal start (by manager) and end (by thread) of a thread
  sc_event evt_;
  //! Event to monitor core status
  sc_event monitor_;
  //! Various mutexes for memory request.
  sc_mutex mtx_teu_request_mem, mtx_cp_request, mtx_payload;
  //! Semaphore with tokens equal to the maximum number of active threads
  sc_semaphore sem_;
  //! Job queue
  MTQueue<std::shared_ptr<PacketDescriptor>> job_queue_;
  //! Store cp access requests
  std::map<std::size_t, std::size_t> cp_requests_;
  //! Notify event for payloads
  sc_event payload_;
  //! Map to store PacketDescriptors received which are requested by their ids.
  std::map<std::size_t, std::shared_ptr<PacketDescriptor>>
      payload_requested_pds;
  //! Map to store payloads received that can be requested using their ids.
  std::map<std::size_t, std::shared_ptr<Packet>> buffer_;
  const char* name;              /*!< Parent name */
  //! Map that stores memory messages received by HAL
  std::map<std::size_t, std::shared_ptr<IPC_MEM>> tlmvar_halreqs_buffer;
  std::map<std::size_t, std::size_t> tlmvar_halreqs;
  //! Event to notify that something received in tlmvarrequests buffers
  sc_event tlmvar_halevent;
  //! Mutex to access tlmvar requests buffer
  sc_mutex tlmvar_halmutex;
  //! HAL TLM utils Decode counter for requests sent.
  int tlmreqcounter;
  //! Core number that this hal belongs to
  std::string core_number;  // TODO(Lemniscate): Change name
  //! Core name
  std::string hal_core_name;
  //! Job Request Counter
  int JobRequestCounter;
  //! Counter Names
  std::string ProcessedPDs;
  std::string AssignedPDs;
  //! Memory Mapping Information
  MemoryUtility meminfo;
};

#endif  // BEHAVIOURAL_HAL_H_
