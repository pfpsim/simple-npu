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
//  RangeTrieNode.cpp
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-05-29.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#include "RangeTrieNode.h"
#include "tlmsingletoninclude.h"

RangeTrieNode::RangeTrieNode() : mComparisons(0), mAddressAlignment(0), mCommonPrefix(0), mCommonPrefixLength(0), mCommonSuffix(0), mCommonSuffixLength(0), mCompareLength(0), mComparePosition(0), mNextNodes(0), mNumOfComparisons(0), mRegionIDs(0) {
  int addr = tlmsingelton::getInstance().tlmvarptr->allocate_mem(32*11); //Allocate and Construct in tlm memory
  std::size_t val =0;
  tlmsingelton::getInstance().tlmvarptr->write_mem(val,this->tlm_addr);
  tlmsingelton::getInstance().tlmvarptr->write_mem(val,this->tlm_addr+1);
  tlmsingelton::getInstance().tlmvarptr->write_mem(val,this->tlm_addr+2);
  tlmsingelton::getInstance().tlmvarptr->write_mem(val,this->tlm_addr+3);
  tlmsingelton::getInstance().tlmvarptr->write_mem(val,this->tlm_addr+4);
  tlmsingelton::getInstance().tlmvarptr->write_mem(val,this->tlm_addr+5);
  tlmsingelton::getInstance().tlmvarptr->write_mem(val,this->tlm_addr+6);
  tlmsingelton::getInstance().tlmvarptr->write_mem(val,this->tlm_addr+7);
  tlmsingelton::getInstance().tlmvarptr->write_mem(val,this->tlm_addr+8);
  tlmsingelton::getInstance().tlmvarptr->write_mem(val,this->tlm_addr+9);
  tlmsingelton::getInstance().tlmvarptr->write_mem(val,this->tlm_addr+10);
}

RangeTrieNode::~RangeTrieNode() {
    if (mComparisons) {
        delete [] mComparisons;
        mComparisons = 0;
    }
    if (mRegionIDs) {
        delete [] mRegionIDs;
        mRegionIDs = 0;
    }
    if (mNextNodes) {
        delete [] mNextNodes;
    }
}


// Getters
unsigned int* RangeTrieNode::getComparisons() const {
  std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr,reinterpret_cast<std::size_t>(mComparisons));
  return mComparisons;
}

unsigned int RangeTrieNode::getNumOfComparisons() const {
  std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+1,static_cast<std::size_t>(mNumOfComparisons));
  return mNumOfComparisons;
}

unsigned int RangeTrieNode::getComparePosition() const {
  std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+2,static_cast<std::size_t>(mComparePosition));
  return mComparePosition;
}

unsigned int RangeTrieNode::getCompareLength() const {
  std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+3,static_cast<std::size_t>(mCompareLength));
  return mCompareLength;
}

int RangeTrieNode::getCommonPrefix() const {
  std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+4,static_cast<std::size_t>(mCommonPrefix));
  return mCommonPrefix;
}

unsigned int RangeTrieNode::getCommonPrefixLength() const {
  std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+5,static_cast<std::size_t>(mCommonPrefixLength));
  return mCommonPrefixLength;
}

int RangeTrieNode::getCommonSuffix() const {
  std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+6,static_cast<std::size_t>(mCommonSuffix));
  return mCommonSuffix;
}

unsigned int RangeTrieNode::getCommonSuffixLength() const {
  std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+7,static_cast<std::size_t>(mCommonSuffixLength));
  return mCommonSuffixLength;
}

bool RangeTrieNode::getAddressAlignment() const {
  std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+8,static_cast<std::size_t>(mAddressAlignment));
  return mAddressAlignment;
}

RangeTrieNode* RangeTrieNode::getNextNodes() const {
  std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+9,reinterpret_cast<std::size_t>(mNextNodes));
  return mNextNodes;
}

int* RangeTrieNode::getRegionIDs() const {
  std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+10,reinterpret_cast<std::size_t>(mRegionIDs));
  return mRegionIDs;
}

// Setters
void RangeTrieNode::setComparisons(unsigned int* iComparisons) {
  tlmsingelton::getInstance().tlmvarptr->write_mem(reinterpret_cast<std::size_t>(iComparisons),this->tlm_addr);
  mComparisons = iComparisons;
}

void RangeTrieNode::setNumOfComparisons(unsigned int iNum) {
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(iNum),this->tlm_addr+1);
  mNumOfComparisons = iNum;
}

void RangeTrieNode::setComparePosition(unsigned int iPos) {
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(iPos),this->tlm_addr+2);
  mComparePosition = iPos;
}

void RangeTrieNode::setCompareLength(unsigned int iLength) {
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(iLength),this->tlm_addr+3);
  mCompareLength = iLength;
}

void RangeTrieNode::setCommonPrefix(int iPrefix) {
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(iPrefix),this->tlm_addr+4);
  mCommonPrefix = iPrefix;
}

void RangeTrieNode::setCommonPrefixLength(unsigned int iLength) {
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(iLength),this->tlm_addr+5);
  mCommonPrefixLength = iLength;
}

void RangeTrieNode::setCommonSuffix(int iSuffix) {
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(iSuffix),this->tlm_addr+6);
  mCommonSuffix = iSuffix;
}

void RangeTrieNode::setCommonSuffixLength(unsigned int iLength) {
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(iLength),this->tlm_addr+7);
  mCommonSuffixLength = iLength;
}

void RangeTrieNode::setAddressAlignment(bool iAlign) {
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(iAlign),this->tlm_addr+8);
  mAddressAlignment = iAlign;
}

void RangeTrieNode::setNextNodes(RangeTrieNode *iNodes) {
  tlmsingelton::getInstance().tlmvarptr->allocate(reinterpret_cast<std::size_t>(iNodes),this->tlm_addr+9);
  mNextNodes = iNodes;
}

void RangeTrieNode::setRegionIDs(int *iIDs) {
  tlmsingelton::getInstance().tlmvarptr->allocate(reinterpret_cast<std::size_t>(iIDs),this->tlm_addr+10);
  mRegionIDs = iIDs;
}

/*
 *------------- Allocation Operators ----------------------------------------------
 */

void* RangeTrieNode::operator new(long unsigned size) throw(const char*) {
  void *p = malloc(size); if (!p) throw "Node::operator new() error";
  return p;
}

void* RangeTrieNode::operator new[](long unsigned size) throw(const char*) {
    void *p = malloc(size); if (!p) throw "Node::operator new[]() error";
    return p;
}
void RangeTrieNode::operator delete(void *p) {
    free(p);
}
void RangeTrieNode::operator delete[](void *p) {
    free(p);
}
