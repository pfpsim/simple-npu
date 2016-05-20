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

#include "./ControlPlaneAgent.h"
#include <string>
#include <vector>
#include "common/P4.h"
#include "common/RoutingPacket.h"
#include "common/PacketDescriptor.h"
#include "common/Packet.h"

ControlPlaneAgent::ControlPlaneAgent(sc_module_name nm,
                  pfp::core::PFPObject* parent,
                 std::string configfile)
  : ControlPlaneAgentSIM(nm, parent, configfile),
    table_name_mutex_(sc_gen_unique_name("mutex_")),
    outlog(OUTPUTDIR+"ControlPlaneAgentTableAllocationTrace.csv") {
    /*sc_spawn threads*/
  sc_spawn(sc_bind(&ControlPlaneAgent::command_processing_thread, this));
}

void ControlPlaneAgent::init() {
  init_SIM(); /* Calls the init of sub PE's and CE's */
  npulog(profile, cout << "Initializing P4" << endl;)

  // TODO(gordon) - figure out how to get a unique name in case of
  // multiple npu instances in one simulation
  P4::get("npu")->init_objects(SPARG("p4"));

  npulog(profile, cout << "P4 init done!" << endl;)
}

std::shared_ptr<pfp::cp::CommandResult>
ControlPlaneAgent::send_command(const std::shared_ptr<pfp::cp::Command> & cmd) {
  npulog(profile, cout << "send command start" << endl;)
  current_command = cmd;

  npulog(profile, cout << "notifying new_command" << endl;)
  new_command.notify();

  npulog(profile, cout << "waiting command_processed" << endl;)
  wait(command_processed);

  return current_result;
}

void ControlPlaneAgent::command_processing_thread() {
  while (true) {
    npulog(profile, cout << "waiting new_command" << endl;)
    wait(new_command);

    npulog(profile, cout << "accepting command" << endl;)
    current_result = accept_command(current_command);

    npulog(profile, cout << "notifying command_processed" << endl;)
    command_processed.notify();
  }
}

/* anonymous */namespace {

std::string to_string(const pfp::cp::Bytes & b) {
  return std::string((const char*)b.data(), b.size());
}

bm::MatchKeyParam to_p4_key(const pfp::cp::MatchKey * k) {
  switch (k->get_type()) {
    case pfp::cp::MatchKey::Type::EXACT:
      return bm::MatchKeyParam(
          bm::MatchKeyParam::Type::EXACT,
          to_string(k->get_data()));

    case pfp::cp::MatchKey::Type::LPM:
      return bm::MatchKeyParam(
          bm::MatchKeyParam::Type::LPM,
          to_string(k->get_data()),
          k->get_prefix_len());

    case pfp::cp::MatchKey::Type::TERNARY:
      return bm::MatchKeyParam(
          bm::MatchKeyParam::Type::TERNARY,
          to_string(k->get_data()),
          to_string(k->get_mask()));

    default:
      std::cerr << "Unknown MatchKey::Type " << k->get_type() << std::endl;
      assert(false);
  }
}

}  // end anonymous namespace

std::shared_ptr<pfp::cp::CommandResult>
ControlPlaneAgent::process(pfp::cp::InsertCommand * cmd) {
  cout << "Insert Command at ControlPlaneAgent" << endl;

  // Convert all of the match keys
  std::vector<bm::MatchKeyParam> keys;
  for (const auto & key : cmd->get_keys()) {
    keys.push_back(to_p4_key(key.get()));
  }

  // Convert all of the action parameters
  bm::ActionData action_data;
  for (const pfp::cp::Bytes & b : cmd->get_action().get_params()) {
    action_data.push_back_action_data
      (bm::Data((const char *)b.data(), b.size()));
  }

  // Insert the entry!
  bm::entry_handle_t handle;
  auto p4 = P4::get("npu");
  p4->lock.write_lock();
  bm::MatchErrorCode rc = p4->mt_add_entry(0,
        cmd->get_table_name(),  keys,
        cmd->get_action().get_name(), action_data,
        &handle);
  p4->lock.write_unlock();

  return cmd->success_result(handle);
}

std::shared_ptr<pfp::cp::CommandResult>
ControlPlaneAgent::process(pfp::cp::ModifyCommand *cmd) {
  cout << "Modify Command at ControlPlaneAgent" << endl;
  return cmd->success_result();
}
std::shared_ptr<pfp::cp::CommandResult>
ControlPlaneAgent::process(pfp::cp::DeleteCommand *cmd) {
  cout << "Delete Command at ControlPlaneAgent" << endl;
  return cmd->success_result();
}

std::shared_ptr<pfp::cp::CommandResult>
ControlPlaneAgent::process(pfp::cp::BootCompleteCommand*) {
  ocn_wr_if->put(make_routing_packet(
        module_name_, "splitter",
        std::make_shared<Packet>(1337, 1337, 1337, std::vector<uint8_t>())));
  npulog(profile, cout << "Notifying splitter to accept packets" << endl;)
  return nullptr;
}

/*
 * holds the P4 table name that it is currently constructing. Used by tlmvar to link memory statistics.
 */
std::string ControlPlaneAgent::getCurrentTableName() {
  std::string table_name;
  table_name_mutex_.lock();
  table_name = current_table_name;
  table_name_mutex_.unlock();
  return table_name;
}
/*
 * used to set the current table name during construction / commit transaction messages
 */
void ControlPlaneAgent::setCurrentTableName(std::string table_name) {
  table_name_mutex_.lock();
  current_table_name = table_name;
  if (table_name != "") {
    auto search = table_mem_usage.find(current_table_name);
    if (search == table_mem_usage.end()) {
      table_mem_usage[current_table_name] = 0;
      add_counter(current_table_name);
    }
    table_name_mutex_.unlock();
  }
  return;
}
// TlmVar updates the map for all successfull allocations for a table.
void ControlPlaneAgent::updateMemUsage(uint64_t addtousage) {
  table_mem_usage[current_table_name] =
      table_mem_usage[current_table_name] + addtousage;
  increment_counter(current_table_name, addtousage);
  outlog << "@" << sc_time_stamp() << ","
         << current_table_name << "," << addtousage << std::endl;
  return;
}
// Reports memusage for a table
uint64_t ControlPlaneAgent::getMemUsage(std::string table_name) {
  return table_mem_usage[table_name];
}
