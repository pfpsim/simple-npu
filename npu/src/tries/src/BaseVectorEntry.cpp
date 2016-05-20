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
//  BaseVectorEntry.cpp
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-02-24.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#include "BaseVectorEntry.h"

/// ==================================
//
//  Constructors
//
/// ==================================

BaseVectorEntry::BaseVectorEntry() {}

BaseVectorEntry::BaseVectorEntry(BitString iPrefix, int iLength,
      int iPrefixTablePtr, int iActionPtr) :
      mPrefix(iPrefix),
      mLength(iLength),
      mPrefixTablePtr(iPrefixTablePtr),
      mActionPtr(iActionPtr) {
}

/// ==================================
//
//  Getters
//
/// ==================================

BitString BaseVectorEntry::getPrefix() const {
    return mPrefix;
}

int BaseVectorEntry::getLength() const {
    return mLength;
}

int BaseVectorEntry::getPrefixTablePtr() const {
    return mPrefixTablePtr;
}

int BaseVectorEntry::getActionPtr() const {
    return mActionPtr;
}

/// ==================================
//
//  Setters
//
/// ==================================

void BaseVectorEntry::setPrefix(BitString iPrefix) {
    mPrefix = iPrefix;
}

void BaseVectorEntry::setLength(int iLength) {
    mLength = iLength;
}

void BaseVectorEntry::setPrefixTablePtr(int iPrefixTablePtr) {
    mPrefixTablePtr = iPrefixTablePtr;
}

void BaseVectorEntry::setActionPtr(int iActionPtr) {
    mActionPtr = iActionPtr;
}
