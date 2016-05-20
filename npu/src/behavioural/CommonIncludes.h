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
 * CommonIncludes.h
 *
 *  Created on: Jan 7, 2016
 *      Author: Lemniscate Snickets
 */

#ifndef BEHAVIOURAL_COMMONINCLUDES_H_
#define BEHAVIOURAL_COMMONINCLUDES_H_

/* STL + STD */
#include <exception>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <numeric>
#include <array>
#include <random>
#include <tuple>
#include <algorithm>
#include <deque>
#include <queue>
#include <list>
#include <limits>
#include <set>
#include <utility>
#include <iterator>

/* Interfaces / Services */
#include "behavioural/HalS.h"

/* Channel Data Structures */
#include "common/PacketDescriptor.h"
#include "common/Packet.h"
#include "common/IPC_MEM.h"
#include "common/InputStimulus.h"

/* Application Layer */
#include "common/ApplicationRegistry.hpp"

/* Memory Utilities */
#include "common/MemoryDefines.h"

/* Pcap Utilities */
#include "common/PcapLogger.h"
#include "common/PcapRepeater.h"

/* Random Crap*/
#include "common/pdcomparison.h"

/* TypeDefs */
typedef std::function<Packet&(Packet&, void*)> application_type;
#endif  // BEHAVIOURAL_COMMONINCLUDES_H_
