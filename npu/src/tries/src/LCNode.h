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
//  LCNode.h
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-02-08.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#ifndef __Trie_Data_Structure__LCNode__
#define __Trie_Data_Structure__LCNode__

#include <stdio.h>
#include <iostream>

using namespace std;

class LCNode {
public:
    // Constructor
    LCNode(int iBranch = 0, int iSkip = 0);

    // Destructor
    virtual ~LCNode();

    // Getters
    int getBranchFactor() const;
    int getSkip() const;
    int getLeftNodePos() const;

    // Setters
    void setBranchFactor(int iBranch);
    void setSkip(int iSkip);
    void setLeftNodePos(int iPos);

    void* operator new(long unsigned) throw(const char*);
    void* operator new[](long unsigned) throw(const char*);
    void operator delete(void*);
    void operator delete[](void*);

private:
    int mBranchFactor;
    int mSkip;
    int mLeftNodePos;
    int tlm_addr;
};

#endif /* defined(__Trie_Data_Structure__LCNode__) */
