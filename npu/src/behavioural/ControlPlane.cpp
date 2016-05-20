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

#include "./ControlPlane.h"
#include <fstream>
#include <string>
#include <vector>
#include "top.h"  // TODO(Lemniscate) Should not explicilty include top

#include "pfpsim/core/cp/CommandParser.h"
#include "pfpsim/core/cp/Commands.h"

ControlPlane::ControlPlane(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):ControlPlaneSIM(nm, parent, configfile) {  // NOLINT(whitespace/line_length)
  /*sc_spawn threads*/
  ThreadHandles.push_back(
    sc_spawn(sc_bind(&ControlPlane::ControlPlaneThread, this, 0)));
  ThreadHandles.push_back(
    sc_spawn(sc_bind(&ControlPlane::command_processing_thread, this)));
}

void ControlPlane::init() {
  init_SIM(); /* Calls the init of sub PE's and CE's */
  top* t = dynamic_cast<top*>(this->parent_);
  t->registerControlPlaneToDebugger(this);
}

void ControlPlane::ControlPlaneThread(std::size_t thread_id) {
  npulog(profile, cout << "Loading configs for all tables" << endl;)

  // Try to open the table population file
  std::string path = SPARG("tpop");
  fstream cfg(path);
  if (!cfg.is_open()) {
    npulog(std::cout << "Failed to open input config " << path << endl;);
    return;
  }

  pfp::cp::CommandParser parser;

  // Read the input file line by line and process each line as a command
  std::string line;
  while (!cfg.eof()) {
    std::getline(cfg, line);
    auto cmd = parser.parse_line(line);
    command_queue.push(cmd);
  }

  command_queue.push(std::make_shared<pfp::cp::BootCompleteCommand>());
  // TODO(gordon) begin transactions
}

void ControlPlane::do_command(std::string cmd_str) {
  pfp::cp::CommandParser parser;
  auto cmd = parser.parse_line(cmd_str);
  command_queue.push(cmd);
}

void ControlPlane::command_processing_thread() {
  while (1) {
    auto cmd = command_queue.pop();
    if (!cmd) continue;
    // Pass the command to ourself so that we can update debugger related state
    accept_command(cmd);
    auto result = cpa->send_command(cmd);
    if (!result) continue;
    // Pass the result of the command to ourself to update debugger state
    accept_result(result);
  }
}

/*
 * The ControlPlane uses the CommandProcessor and ResultProcessor interfaces to notify the debugger about changes
 */
// For pfp::cp::CommandProcessor
std::shared_ptr<pfp::cp::CommandResult> ControlPlane::process
    (pfp::cp::InsertCommand *cmd) {
  std::string table_name = cmd->get_table_name();
  // Create a string of all of the keys and store it
  std::stringstream keyss;
  for (auto & k : cmd->get_keys()) {
    keyss << *k << "  ";
  }
  std::string keys = keyss.str();

  std::string action = cmd->get_action().get_name();

  auto params = cmd->get_action().get_params();
  std::vector<std::string> param_strings;
  for (const pfp::cp::Bytes & p : params) {
    std::stringstream ss;
    ss << p;
    param_strings.push_back(ss.str());
  }

  addTableEntry(table_name, keys, action, param_strings);

  return nullptr;  // We're not checking this result
}

std::shared_ptr<pfp::cp::CommandResult>
ControlPlane::process(pfp::cp::ModifyCommand *cmd) {
  std::string table_name = cmd->get_table_name();
  uint64_t handle = cmd->get_handle();
  std::string action = cmd->get_action().get_name();
  auto params = cmd->get_action().get_params();
  std::vector<std::string> param_strings;
  for (const pfp::cp::Bytes & p : params) {
    std::stringstream ss;
    ss << p;
    param_strings.push_back(ss.str());
  }

  updateTableEntry(table_name, handle, action, param_strings);

  return nullptr;
}

std::shared_ptr<pfp::cp::CommandResult>
ControlPlane::process(pfp::cp::DeleteCommand *cmd) {
  deleteTableEntry(cmd->get_table_name(), cmd->get_handle());
  return nullptr;
}
std::shared_ptr<pfp::cp::CommandResult>
ControlPlane::process(pfp::cp::BootCompleteCommand*) {
  return nullptr;
}

// For cp::ResultProcessor
void ControlPlane::process(pfp::cp::InsertResult *res) {
  auto cmd = dynamic_cast<pfp::cp::InsertCommand*>(res->command.get());
  assert(cmd);
  std::string table_name = cmd->get_table_name();
  // Create a string of all of the keys and store it
  std::stringstream keyss;
  for (auto & k : cmd->get_keys()) {
    keyss << *k << "  ";
  }
  std::string keys = keyss.str();
  std::string action = cmd->get_action().get_name();
  updateHandle(table_name, keys, action, res->handle);
}

void ControlPlane::process(pfp::cp::ModifyResult *res) {
  auto cmd = dynamic_cast<pfp::cp::ModifyCommand*>(res->command.get());
  assert(cmd);
  std::string table_name = cmd->get_table_name();
  uint64_t handle = cmd->get_handle();
  confirmUpdateEntry(table_name, handle);
}

void ControlPlane::process(pfp::cp::DeleteResult *res) {
  auto cmd = dynamic_cast<pfp::cp::DeleteCommand*>(res->command.get());
  assert(cmd);
  std::string table_name = cmd->get_table_name();
  uint64_t handle = cmd->get_handle();
  confirmDeleteEntry(table_name, handle);
}

void ControlPlane::process(pfp::cp::FailedResult*) {
  // TODO(gordon)
}
