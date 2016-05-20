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
 * TlmVar.h
 *
 *  Created on: June 21, 2015
 *      Author: Lemniscate Snickets
 */

#ifndef BEHAVIOURAL_COMMON_TLMVAR_H_
#define BEHAVIOURAL_COMMON_TLMVAR_H_

#include <string>

class tlmvar {
 public:
  tlmvar();
  virtual ~tlmvar() = default;

  // TLM Memory related functions refactor for new tries
  std::size_t allocate_mem(int size_of_data);  // get address
  void allocate(std::size_t data_to_allocate, int addr);  // write function
  std::size_t read_mem(int addr, std::size_t val_compare = 0);  // read function

  // Wrapper for allocate to not break legacy
  void write_mem(std::size_t data_to_allocate, int addr);

  unsigned long wordsizetoBytes(std::size_t wordsize);  // NOLINT

  typedef int tlm_addr_;
  struct memdecode{
    tlm_addr_ physcialaddr;
    std::string memname;
  };
  static std::string module_name_;
};
#endif  // BEHAVIOURAL_COMMON_TLMVAR_H_
