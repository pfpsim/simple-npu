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

#ifndef BEHAVIOURAL_CONTROLPLANEAGENTHALS_H_
#define BEHAVIOURAL_CONTROLPLANEAGENTHALS_H_

#include "systemc.h"  // NOLINT
#include "tlm.h"  // NOLINT
using tlm::tlm_tag;

class ControlPlaneAgentHalS : public sc_interface {
 public:
    /* User Logic - Virtual Functions for interface go here */
    typedef std::size_t TlmType;
    virtual void tlmwrite(int VirtualAddress, int data, TlmType size) = 0;
    virtual TlmType tlmread(int VirtualAddress) = 0;
    virtual TlmType tlmallocate(int BytestoAllocate) = 0;
};
#endif  // BEHAVIOURAL_CONTROLPLANEAGENTHALS_H_
