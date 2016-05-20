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
 * TestApplication.cpp
 *
 *  Created on: Aug 15, 2014
 *      Author: kamil
 */
/**
 * @file TestApplication
 * This file contains a test application to verify the accuracy of the DICP profiling tool
 */

#include <iostream>
#include "ApplicationRegistry.hpp"
#include "P4.h"

/**
 * Test application with various branches containing nonsense instructions of different sizes
 * @param counter	FOR PROFILING ONLY: holds the number of assembly instructions executed when the application is complete
 * @param np		PacketDescriptor to process
 * @param p			Packet to process
 * @param args		Additional arguments -- NOTE: These must be correctly recast by the application programmer
 * @return			Processed Packet
 */
Packet& P4Application(uint32_t &counter,
    PacketDescriptor &np, Packet &p, void *args = 0) {
  static auto p4 = P4::get("npu");
  static auto ingress = p4->get_pipeline("ingress");
  static auto egress  = p4->get_pipeline("egress");
  auto p4_packet = np.header().get();
  p4->lock.read_lock();
  ingress->apply(p4_packet);
  // TODO(gordon) if( ! np.drop()){
  egress->apply(p4_packet);
  p4->lock.read_unlock();
  /*
  // We re-attach the full payload for the table application, incase it is
  // needed by the P4 program (to calculate a TCP checksum, for example)
  np.swapPayload(p.data().data(), p.data().size(), &payload, &length);
  bool ret = p4_do_table_application(np.header().get());
  // One processing is complete, we restore the original empty payload
  np.swapPayload(payload, length);
  */
  return p;
}

static int ts = register_application("P4Application", P4Application);
