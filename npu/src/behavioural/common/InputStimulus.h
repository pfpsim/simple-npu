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
 * InputStimulus.h
 *
 *  Created on: Feb 7, 2016
 *      Author: Lemniscate Snickets
 */

#ifndef BEHAVIOURAL_COMMON_INPUTSTIMULUS_H_
#define BEHAVIOURAL_COMMON_INPUTSTIMULUS_H_

#include <tuple>
#include <string>
#include "pfpsim/pfpsim.h"
#include "Packet.h"

typedef std::tuple<std::shared_ptr<Packet>, std::size_t, std::size_t>
        ingress_stimulus_type;

class InputStimulus: public pfp::core::TrType {
 public:
  InputStimulus(std::size_t id, ingress_stimulus_type input);
  InputStimulus(const InputStimulus & other) = default;

  virtual ~InputStimulus() = default;

  std::string data_type() const;

  ingress_stimulus_type stimulus_object();

 private:
  ingress_stimulus_type packet;
};

#endif  // BEHAVIOURAL_COMMON_INPUTSTIMULUS_H_
