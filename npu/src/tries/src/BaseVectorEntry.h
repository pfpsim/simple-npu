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
//  BaseVectorEntry.h
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-02-24.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#ifndef __TRIE__BASEVECTORENTRY__
#define __TRIE__BASEVECTORENTRY__

#include <iostream>
#include "BitString.h"

class BaseVectorEntry {
 public:
    // Constructors
    BaseVectorEntry();
    BaseVectorEntry(BitString iPrefix, int iLength,
        int iPrefixTablePtr, int iActionPtr = 0);

    // Getters
    BitString getPrefix() const;
    int getLength() const;
    int getPrefixTablePtr() const;
    int getActionPtr() const;

    // Setters
    void setPrefix(BitString iPrefix);
    void setLength(int iLength);
    void setPrefixTablePtr(int iPrefixTablePtr);
    void setActionPtr(int iActionPtr);

 private:
    BitString mPrefix;
    int mLength;
    int mPrefixTablePtr;
    int mActionPtr;
};

#endif  // __TRIE__BASEVECTORENTRY__
