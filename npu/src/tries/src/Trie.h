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

//
//  Trie.h
//

#ifndef __Trie__
#define __Trie__

#include <string>
#include "RoutingTableEntry.h"
#include "BitString.h"

using namespace std;

template <class T>
class Trie {
public:

    // Enumeration
    enum Action {
        Reconstruct,
        Add,
        Remove
    };

    // Constructor
    Trie();

    // Destructor
    virtual ~Trie();

    // Update
    virtual void update(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, Action iAction) = 0;

    // Lookup
    virtual T exactPrefixMatch(BitString iPrefix) const = 0;
    virtual T longestPrefixMatch(BitString iPrefix) const = 0;

};

//
//  Trie.cpp
//

//#include "Trie.h"

/// ============================
//
//  Constructor
//
/// ============================

template <class T>
Trie<T>::Trie() {}

/// ============================
//
//  Destructor
//
/// ============================

template <class T>
Trie<T>::~Trie() {}

#endif /* defined(__Trie__) */
