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

#ifndef BEHAVIOURAL_NPU_H_
#define BEHAVIOURAL_NPU_H_
#include <string>
#include <vector>
#include <map>
#include "../structural/NPUSIM.h"

#include "pfpsim/core/cp/Commands.h"

class NPU: public NPUSIM {
 public:
  SC_HAS_PROCESS(NPU);
  /*Constructor*/
  explicit NPU(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~NPU() = default;

 public:
  void init();

 public:
  // For ControlPlaneAgentS
  std::shared_ptr<pfp::cp::CommandResult>
  send_command(const std::shared_ptr<pfp::cp::Command> & cmd) override;

 private:
  //! Internal list of submodules
  std::map<std::string, pfp::core::PFPObject*> common_interface_;
  // TODO(eric) test with debugger
};

#endif  // BEHAVIOURAL_NPU_H_
