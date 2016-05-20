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

#include "./top.h"
#include <string>

top::top(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):topSIM(nm, parent, configfile), DebuggerUtilities() {  // NOLINT(whitespace/line_length)
    /*sc_spawn threads*/
    ThreadHandles.push_back(
      sc_spawn(
        sc_bind(&top::notify_observers, this, this)));
}

top::~top() {
}

void top::init() {
#ifdef PFPSIM_DEBUGGER
  attach_observer(debug_obs);
  if (PFP_DEBUGGER_ENABLED) {
    debug_obs->enableDebugger();
    debug_obs->waitForRunCommand();
  }
#endif

#ifndef PFPSIM_DEBUGGER
  if (PFP_DEBUGGER_ENABLED) {
    std::cerr << "\n" << On_IRed
              << "ERROR: Please compile with PFPSIM_DEBUGGER flag for access to"
              << " the debugger. Use: cmake -DPFPSIMDEBUGGER=ON ../src/ in the"
              << " build directory." << txtrst << std::endl;
    return 1;
  }
#endif
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
