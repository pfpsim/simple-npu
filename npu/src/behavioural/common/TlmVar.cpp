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
 * TlmVar.cpp
 *
 *  Created on: July 21, 2015
 *      Author: Lemniscate Snickets
 */


#include <iostream>
#include <string>
#include <vector>

#include "TlmVar.h"
#include "tlmsingleton.h"  // NOLINT
#include "../HalS.h"
#include "../ControlPlaneAgentHalS.h"
#include "../ControlPlaneAgentHAL.h"
#include "../HAL.h"
#include "../ControlPlaneAgent.h"
#include "../QueueWrI.h"
#include "routingdefs.h" // NOLINT


typedef HalS HALInterface;

// NOLINTNEXTLINE(runtime/string)
std::string tlmvar::module_name_ = "TLMVAR";  // hack for npulog

tlmvar::tlmvar() {
}

std::size_t tlmvar::allocate_mem(int size_of_data) {
  sc_process_handle this_process = sc_get_current_process_handle();
  sc_object* current_scmodule = this_process.get_parent_object();
  const char* currentmodulename = current_scmodule->basename();
  npulog(cout << "&@--Allocate_mem--->" << currentmodulename << endl;)

  std::vector<sc_object *> children = current_scmodule->get_child_objects();
  std::vector<sc_event *> children_events
    = current_scmodule->get_child_events();
  std::vector<std::string> results;
  std::vector<std::string> results_events;

  if (std::string(currentmodulename).find("applayer") !=std::string::npos) {
    sc_port<HALInterface>* halport = nullptr;
    npulog(cout << "In Application Layer" << endl;)
    SC_REPORT_ERROR("TLMVAR", "HALT-AllocateMEM TEU - Not Implemented");
    sc_stop();
    // End Brace if ApplicationLayer
  } else if (std::string(currentmodulename).find(ControlPlaneAgentName)
              !=std::string::npos) {
    npulog(cout << "TLMVAR - In ControlPlaneAgent" << endl;)
    if (dynamic_cast<ControlPlaneAgent*>(current_scmodule)) {
      // 1. Get Pointer to CPAGENT
      ControlPlaneAgent* cpagentptr;
      cpagentptr = dynamic_cast<ControlPlaneAgent*>(current_scmodule);
      // 2. Call the HAL Function to Allocate
      // Todo: Dont hardcode it
      std::size_t AllocatedVirtualAddress
          = cpagentptr->cpagenthal->tlmallocate(size_of_data);
      npulog(cout << "TlmVar Allocated@:" << AllocatedVirtualAddress << endl;)
      return AllocatedVirtualAddress;
    } else {
      SC_REPORT_ERROR("TLMVAR", "CP-Agent Dynamic-Cast failed TlmVar::Allocate()");  // NOLINT
      sc_stop();
    }
  // End brace if module is CP_Agent
  } else {
    std::cerr << "TlmVar Allocate not configured for: "
              << currentmodulename << endl;
    SC_REPORT_ERROR("TlmVar", "Inside Unknown Module");
    sc_stop();
  }
}

void tlmvar::write_mem(std::size_t data_to_allocate, int addr) {
  allocate(data_to_allocate, addr);
  return;
}
// Write function
void tlmvar::allocate(std::size_t data_to_allocate, int addr) {
  sc_process_handle this_process = sc_get_current_process_handle();
  sc_object* current_scmodule = this_process.get_parent_object();
  const char* currentmodulename = current_scmodule->basename();
  npulog(cout << "&@--Write_mem--->" << currentmodulename
              << " write to vaddr:" << addr << endl;)
  if (std::string(currentmodulename).find(ApplicationLayerName)
          !=std::string::npos) {
    npulog(cout << "TLMVAR In Application Layer" << endl;)
    SC_REPORT_ERROR("TLMVAR", "HALT-WriteMEM TEU - Not Implemented");
    sc_stop();
  // End Brace if ApplicationLayer
  } else if (std::string(currentmodulename).find(ControlPlaneAgentName)
              !=std::string::npos) {
    npulog(cout << "TLMVAR In ControlPlaneAgent" << endl;)
    if (dynamic_cast<ControlPlaneAgent*>(current_scmodule)) {
      // 1. Get Pointer to CPAGENT
      ControlPlaneAgent* cpagentptr;
      cpagentptr = dynamic_cast<ControlPlaneAgent*>(current_scmodule);
      // 2. Call the HAL Function to Wrtie
      // Todo: Dont hardcode it
      cpagentptr->cpagenthal->tlmwrite(addr, data_to_allocate, 0);
      return;
    } else {
      std::cerr
      << " TLMVAR Write to mem module ControlPlaneAgent Dynamic cast failed"
      << endl;
      sc_stop();
    }
  // End brace if module is CP_Agent
  } else {
    std::cerr
    << "TlmVar - I should not be here - does not support: "
    << currentmodulename << endl;
    SC_REPORT_ERROR("TlmVar", "Inside Unknown Module");
    sc_stop();
  }
}

std::size_t tlmvar::read_mem(int addr, std::size_t val_compare) {
  sc_process_handle this_process = sc_get_current_process_handle();
  sc_object* current_scmodule = this_process.get_parent_object();
  const char* currentmodulename = current_scmodule->basename();
  npulog(cout << "&@--Read_mem--->" << currentmodulename << endl;)
  std::vector<sc_object *> children = current_scmodule->get_child_objects();
  std::vector<sc_event *> children_events
      = current_scmodule->get_child_events();
  std::vector<std::string> results;
  std::vector<std::string> results_events;

  if (std::string(currentmodulename).find(ApplicationLayerName)
        !=std::string::npos) {
    sc_port<HALInterface>* halport = nullptr;
    npulog(cout << "TLMVAR -read mem- In Application Layer" << endl;)
    for (auto& each_child : children) {
      if (dynamic_cast<sc_port<HALInterface>*>(each_child)) {
        halport = dynamic_cast<sc_port<HALInterface>*>(each_child);
        npulog(std::cout <<  "Write Port: " << halport->kind()
                         << " portname:" << halport->name() <<  std::endl;)
      }
    }
    int data = 0xDEADBEEF;
    std::size_t size = 0;
    std::size_t dataval
        = halport->get_interface(0)->tlmread(addr, data, size, val_compare);
    npulog(cout << "TlmVar TEUHAL read@ " << addr << " data:" << data << endl;)
    return dataval;
    // End Brace if ApplicationLayer
  } else if (std::string(currentmodulename).find(ControlPlaneAgentName)
                !=std::string::npos) {
    npulog(cout << "TLMVAR In ControlPlaneAgent" << endl;)
    if (dynamic_cast<ControlPlaneAgent*>(current_scmodule)) {
      ControlPlaneAgent* cpagentptr;
      cpagentptr = dynamic_cast<ControlPlaneAgent*>(current_scmodule);
      // 2. Call the HAL Function to Wrtie
      // Todo: Dont hardcode it
      std::size_t dataval = cpagentptr->cpagenthal->tlmread(addr);
      return dataval;
    } else {
      std::cerr << "TLMVAR ControlPlaneAgent Dynamic cast failed" << endl;
      sc_stop();
    }
  // End brace if module is CP_Agent
  } else {
    std::cerr << "TlmVar doesnot support: " << currentmodulename << endl;
    SC_REPORT_ERROR("TlmVar", "Inside Unknown Module");
    sc_stop();
  }
}

unsigned long tlmvar::wordsizetoBytes(std::size_t wordsize) { // NOLINT
  return ceil((float) wordsize / (float) 32); // NOLINT
}
