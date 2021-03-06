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

#ifndef BEHAVIOURAL_SORTEDLOGGER_H_
#define BEHAVIOURAL_SORTEDLOGGER_H_
#include <string>
#include <vector>
#include "../structural/SortedLoggerSIM.h"
#include "common/Packet.h"

class SortedLogger: public SortedLoggerSIM {
 public:
  SC_HAS_PROCESS(SortedLogger);
  /*Constructor*/
  explicit SortedLogger(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~SortedLogger();

 public:
  void init();

 private:
  void SortedLogger_PortServiceThread();
  void SortedLoggerThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  std::vector<std::shared_ptr<Packet>> packets;
  std::string logfilename;
};

#endif  // BEHAVIOURAL_SORTEDLOGGER_H_
