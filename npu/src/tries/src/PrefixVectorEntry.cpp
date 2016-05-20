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
//  PrefixVectorEntry.cpp
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-02-24.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#include "PrefixVectorEntry.h"

/// ==================================
//
//  Constructors
//
/// ==================================

PrefixVectorEntry::PrefixVectorEntry() {}

PrefixVectorEntry::PrefixVectorEntry(int iLength, int iPrefixPtr, int iActionPtr) : mLength(iLength), mPrefixPtr(iPrefixPtr), mActionPtr(iActionPtr) {

}

/// ==================================
//
//  Getters
//
/// ==================================

int PrefixVectorEntry::getLength() const {
    return mLength;
}

int PrefixVectorEntry::getPrefixPtr() const {
    return mPrefixPtr;
}

int PrefixVectorEntry::getActionPtr() const {
    return mActionPtr;
}

/// ==================================
//
//  Setters
//
/// ==================================

void PrefixVectorEntry::setLength(int iLength) {
    mLength = iLength;
}

void PrefixVectorEntry::setPrefixPtr(int iPrefixPtr) {
    mPrefixPtr = iPrefixPtr;
}

void PrefixVectorEntry::setActionPtr(int iActionPtr) {
    mActionPtr = iActionPtr;
}