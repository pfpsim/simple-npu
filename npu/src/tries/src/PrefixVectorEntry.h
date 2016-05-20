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
//  PrefixVectorEntry.h
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-02-24.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#ifndef __Trie_Data_Structure__PrefixVectorEntry__
#define __Trie_Data_Structure__PrefixVectorEntry__

#include <iostream>

class PrefixVectorEntry {
public:
    // Constructors
    PrefixVectorEntry();
    PrefixVectorEntry(int iLength, int iPrefixPtr, int iActionPtr = 0);

    //Getters
    int getLength() const;
    int getPrefixPtr() const;
    int getActionPtr() const;

    //Setters
    void setLength(int iLength);
    void setPrefixPtr(int iPrefixPtr);
    void setActionPtr(int iActionPtr);

private:
    int mLength;
    int mPrefixPtr;
    int mActionPtr;
};

#endif /* defined(__Trie_Data_Structure__PrefixVectorEntry__) */