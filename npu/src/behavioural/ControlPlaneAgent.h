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

#ifndef BEHAVIOURAL_CONTROLPLANEAGENT_H_
#define BEHAVIOURAL_CONTROLPLANEAGENT_H_
#include <string>
#include <vector>
#include <map>
#include "../structural/ControlPlaneAgentSIM.h"
#include "pfpsim/core/cp/Commands.h"
#include "common/PacketDescriptor.h"
#include "common/P4.h"


class ControlPlaneAgent:
  public ControlPlaneAgentSIM,
  public pfp::cp::CommandProcessor {
 public:
  SC_HAS_PROCESS(ControlPlaneAgent);
  /*Constructor*/
  explicit ControlPlaneAgent(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT(whitespace/line_length)
  /*Destructor*/
  virtual ~ControlPlaneAgent() = default;

 public:
  void init();

  std::shared_ptr<pfp::cp::CommandResult>
  send_command(const std::shared_ptr<pfp::cp::Command> & cmd) override;

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

  void command_processing_thread();

 private:
  sc_event new_command;
  std::shared_ptr<pfp::cp::Command> current_command;

  sc_event command_processed;
  std::shared_ptr<pfp::cp::CommandResult> current_result;
 public:
  //! Replies from ControlPlane
  bool handlePacketFromControlPlane(PacketDescriptor & packet);

/**
 *  holds the P4 table name that it is currently constructing. Used by tlmvar to link memory statistics.
 * @return current table under processing
 */
  std::string getCurrentTableName();
  /**
   * used to set the current table name during construction / commit transaction messages.
   * @param table_name current table processing
   */
  void setCurrentTableName(std::string table_name);
  /**
   * TlmVar updates the map for all successfull allocations for a table.
   * @param addtousage Allocation size
   */
  void updateMemUsage(uint64_t addtousage);
  /**
   *  Reports memusage for a table
   * @param  std::string table name
   * @return             memusage in bytes
   */
  uint64_t getMemUsage(std::string);

 private:
  //! Internal representation to keep track of TECs requested Control plane
  std::map<std::size_t, std::string> cp_requested;
  //! Mutex for TECs requests buffer to Control plane
  sc_mutex mtx_cp_requested_;
  //! Map that stores memory usage by table name
  std::map<std::string, uint64_t> table_mem_usage;
  //! current table that the agent is processing
  std::string current_table_name;
  //! Mutex guard for acessing table name
  sc_mutex table_name_mutex_;
  //! ostream for logging to file
  std::ofstream outlog;

  //! Holds all currently queued insert operations for a transaction
  struct TransactionInfo {
    std::vector<std::vector<bm::MatchKeyParam> > keys;
    std::vector<std::string> actions;
    std::vector<bm::ActionData> action_data;
    std::vector<bm::entry_handle_t*> handles;
  };

  //! Holds all of the information for all of the current transactions,
  //! Organized by table
  std::map<std::string, TransactionInfo> transaction;

  //! Indicated that we're currently inside a transaction
  bool in_transaction;

};

#endif  // BEHAVIOURAL_CONTROLPLANEAGENT_H_
