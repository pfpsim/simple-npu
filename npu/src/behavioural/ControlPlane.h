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

#ifndef BEHAVIOURAL_CONTROLPLANE_H_
#define BEHAVIOURAL_CONTROLPLANE_H_
#include <string>
#include <vector>
#include "../structural/ControlPlaneSIM.h"
#include "pfpsim/pfpsim.h"

// TODO(gordon) namespace pfp {

class ControlPlane: public ControlPlaneSIM,
          public pfp::core::db::CPDebuggerInterface,
          public pfp::cp::CommandProcessor,
          public pfp::cp::ResultProcessor {
 public:
  SC_HAS_PROCESS(ControlPlane);
  /*Constructor*/
  explicit ControlPlane(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~ControlPlane() = default;
 public:
  void init();

  // Implementing methods from CPDebuggerInterface
  void do_command(std::string cmd) override;

  // For pfp::cp::CommandProcessor
  std::shared_ptr<pfp::cp::CommandResult>
       process(pfp::cp::InsertCommand*) override;
  std::shared_ptr<pfp::cp::CommandResult>
       process(pfp::cp::ModifyCommand*) override;
  std::shared_ptr<pfp::cp::CommandResult>
       process(pfp::cp::DeleteCommand*) override;
  std::shared_ptr<pfp::cp::CommandResult>
       process(pfp::cp::BootCompleteCommand*) override;
  std::shared_ptr<pfp::cp::CommandResult>
       process(pfp::cp::BeginTransactionCommand*) override;
  std::shared_ptr<pfp::cp::CommandResult>
       process(pfp::cp::EndTransactionCommand*) override;

  // For pfp::cp::ResultProcessor
  void process(pfp::cp::InsertResult*) override;
  void process(pfp::cp::ModifyResult*) override;
  void process(pfp::cp::DeleteResult*) override;
  void process(pfp::cp::FailedResult*) override;
 private:
  void ControlPlaneThread(std::size_t thread_id);
  void command_processing_thread();
  void _insert_entry(std::string table_name, std::string match_key,
        std::string action_name, std::vector<std::string> action_data);
  void _delete_entry(std::string table_name, uint64_t handle);
  void _modify_entry(std::string table_name, uint64_t handle,
        std::string action_name, std::vector<std::string> action_data);
  std::vector<sc_process_handle> ThreadHandles;

  MTQueue<std::shared_ptr<pfp::cp::Command> > command_queue;
};

#endif  // BEHAVIOURAL_CONTROLPLANE_H_
