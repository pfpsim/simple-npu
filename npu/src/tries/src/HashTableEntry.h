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
//  HashTableEntry.h
//

#ifndef __Trie_Data_Structure__HashTableEntry__
#define __Trie_Data_Structure__HashTableEntry__

#include <functional>
#include "Trie.h"
#include "systemc.h"
#include "tlmsingletoninclude.h"

template <class T>
class HashTableEntry {
public:
    // Constructor
    HashTableEntry(bool iFlag, unsigned int iKey_Bit, int iKeyLength, T iVal, HashTableEntry<T>* iPtr);
    HashTableEntry();
    HashTableEntry(const HashTableEntry &iCopy);

    // Destructor
    ~HashTableEntry();

    // Getters
    bool getFlag() const;
    unsigned int getKeyBitmap() const;
    T getValue() const;
    HashTableEntry<T>* getPtr() const;
    short getKeyLength() const;
    int getValPtrSize() const;

    // Setters
    void setFlag(bool iFlag);
    void setKeyBitmap(int iKey_Bit);
    void setValue(T iVal, int iSize);
    void setPtr(HashTableEntry<T>* iPtr, int iSize);
    void setKeyLength(short iLength);

    void* operator new(long unsigned) throw(const char*);
    void* operator new[](long unsigned) throw(const char*);
    void operator delete(void*);
    void operator delete[](void*);


private:
    bool mEntryTypeFlag; // 1 - key/value pair, 0 - bitmap/pointer pair

    unsigned int mKey_Bitmap;
    HashTableEntry<T>* mPtr;

    short mKeyLength;
    T mValue;

    int mVal_PtrSize;

    int tlm_addr;

};


//
//  HashTableEntry.cpp
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-03-25.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

//#include "HashTableEntry.h"

/// ==========================
//
//  Constructors
//
/// ==========================

template <class T>
HashTableEntry<T>::HashTableEntry(bool iFlag, unsigned int iKey_Bit, int iKeyLength, T iVal, HashTableEntry<T>* iPtr) :
mEntryTypeFlag(iFlag), //0
mKey_Bitmap(iKey_Bit), //1
mValue(iVal), //2
mPtr(iPtr), //3
mKeyLength(iKeyLength) //4
{
  int addr = tlmsingelton::getInstance().tlmvarptr->allocate_mem(32*5); //Allocate and Construct in tlm memory
  this->tlm_addr = addr;
  std::size_t val = 0;
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(iFlag),this->tlm_addr);
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(iKey_Bit),this->tlm_addr+1);
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(val),this->tlm_addr+2);
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(val),this->tlm_addr+3);
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(iKeyLength),this->tlm_addr+4);
}

template <class T>
HashTableEntry<T>::HashTableEntry() : mEntryTypeFlag(0), mKey_Bitmap(0), mKeyLength(0), mPtr(0) {
  int addr = tlmsingelton::getInstance().tlmvarptr->allocate_mem(32*5); //Allocate and Construct in tlm memory
  this->tlm_addr = addr;
  std::size_t val = 0;
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(val),this->tlm_addr);
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(val),this->tlm_addr+1);
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(val),this->tlm_addr+2);
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(val),this->tlm_addr+3);
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(val),this->tlm_addr+4);
}

/// ==========================
//
//  Copy Constructor
//
/// ==========================

template <class T>
HashTableEntry<T>::HashTableEntry(const HashTableEntry<T>& iCopy) : mEntryTypeFlag(iCopy.getFlag()), mKey_Bitmap(iCopy.getKeyBitmap()), mKeyLength(iCopy.getKeyLength()), mValue(iCopy.getValue()), mPtr(iCopy.getPtr()) {}

/// ==========================
//
//  Destructor
//
/// ==========================

template <class T>
HashTableEntry<T>::~HashTableEntry() {
/*    if (mEntryTypeFlag == 0 && mPtr != 0) {
        delete [] mPtr;
        mPtr = 0;
    }
*/
}

/// ==========================
//
//  Getters
//
/// ==========================

template <class T>
bool HashTableEntry<T>::getFlag() const {
  std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr);
  // return static_cast<bool>(val);
  return mEntryTypeFlag;
}

template <class T>
unsigned int HashTableEntry<T>::getKeyBitmap() const {
  std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+1);
  return mKey_Bitmap;
  return static_cast<unsigned int>(val);
}

template <class T>
T HashTableEntry<T>::getValue() const {
  std::size_t val = tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+2);
  return mValue;
}

template <class T>
HashTableEntry<T>* HashTableEntry<T>::getPtr() const {
  std::size_t val = tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+3);
  return mPtr;
}

template <class T>
short HashTableEntry<T>::getKeyLength() const {
  std::size_t val = tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+4);
  return mKeyLength;
  return static_cast<short>(val);
}

template <class T>
int HashTableEntry<T>::getValPtrSize() const {
    return mVal_PtrSize;
}

/// ==========================
//
//  Setters
//
/// ==========================

template <class T>
void HashTableEntry<T>::setFlag(bool iFlag) {

    mEntryTypeFlag = iFlag;
    tlmsingelton::getInstance().tlmvarptr->allocate(static_cast<std::size_t>(iFlag),this->tlm_addr);
}

template <class T>
void HashTableEntry<T>::setKeyBitmap(int iKey_Bit) {
    mKey_Bitmap = iKey_Bit;
    tlmsingelton::getInstance().tlmvarptr->allocate(static_cast<std::size_t>(iKey_Bit),this->tlm_addr+1);

}

template <class T>
void HashTableEntry<T>::setValue(T iVal, int iSize) {
    mValue = iVal;
    mVal_PtrSize = iSize;
    tlmsingelton::getInstance().tlmvarptr->allocate(0,this->tlm_addr+2);
}

template <class T>
void HashTableEntry<T>::setPtr(HashTableEntry<T>* iPtr, int iSize) {
    mPtr = iPtr;
    mVal_PtrSize = iSize;
    tlmsingelton::getInstance().tlmvarptr->allocate(0,this->tlm_addr+3);

}

template <class T>
void HashTableEntry<T>::setKeyLength(short iLength) {
    mKeyLength = iLength;
    tlmsingelton::getInstance().tlmvarptr->allocate(static_cast<std::size_t>(iLength),this->tlm_addr+4);
}

template <class T>
void* HashTableEntry<T>::operator new(long unsigned size) throw(const char*) {
#if debug_node_new
    cout << "   Node::operator new(" << size << ")\t";
#endif
    void *p = malloc(size); if (!p) throw "Node::operator new() error";
#if debug_node_new
    cout << static_cast<void*>(p) << endl;
#endif
    return p;
}

template <class T>
void* HashTableEntry<T>::operator new[](long unsigned size) throw(const char*) {
    // cout << "   Node::operator new[](" << size << ")\t";
    void *p = malloc(size); if (!p) throw "Node::operator new[]() error";
    // cout << static_cast<void*>(p) << endl;
    return p;
}
template <class T>
void HashTableEntry<T>::operator delete(void *p) {

}
template <class T>
void HashTableEntry<T>::operator delete[](void *p) {

}
#endif /* defined(__Trie_Data_Structure__HashTableEntry__) */
