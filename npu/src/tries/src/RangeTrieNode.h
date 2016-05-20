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
//  RangeTrieNode.h
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-05-29.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#ifndef __Trie_Data_Structure__RangeTrieNode__
#define __Trie_Data_Structure__RangeTrieNode__

#include <string>
#include "systemc.h"
class RangeTrieNode {
public:

    RangeTrieNode();

    ~RangeTrieNode();

    // Getters
    unsigned int* getComparisons() const;
    unsigned int getNumOfComparisons() const;
    unsigned int getComparePosition() const;
    unsigned int getCompareLength() const;
    int getCommonPrefix() const;
    unsigned int getCommonPrefixLength() const;
    int getCommonSuffix() const;
    unsigned int getCommonSuffixLength() const;
    bool getAddressAlignment() const;
    RangeTrieNode* getNextNodes() const;
    int* getRegionIDs() const;

    // Setters
    void setComparisons(unsigned int* iComparisons);
    void setNumOfComparisons(unsigned int iNum);
    void setComparePosition(unsigned int iPos);
    void setCompareLength(unsigned int iLength);
    void setCommonPrefix(int iPrefix);
    void setCommonPrefixLength(unsigned int iLength);
    void setCommonSuffix(int iSuffix);
    void setCommonSuffixLength(unsigned int iLength);
    void setAddressAlignment(bool iAlign);
    void setNextNodes(RangeTrieNode *iNodes);
    void setRegionIDs(int *iIDs);


    void* operator new(long unsigned) throw(const char*);
    void* operator new[](long unsigned) throw(const char*);
    void operator delete(void*);
    void operator delete[](void*);


private:
    unsigned int *mComparisons;
    unsigned int mNumOfComparisons;
    unsigned int mComparePosition;
    unsigned int mCompareLength;
    int mCommonPrefix;
    unsigned int mCommonPrefixLength;
    int mCommonSuffix;
    unsigned int mCommonSuffixLength;
    bool mAddressAlignment;
    RangeTrieNode *mNextNodes;
    int *mRegionIDs;
    int tlm_addr;
};

#endif /* defined(__Trie_Data_Structure__RangeTrieNode__) */
