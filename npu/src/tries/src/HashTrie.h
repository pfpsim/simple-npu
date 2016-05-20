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
//  HashTrie.h
//

#ifndef __Trie_Data_Structure__HashTrie__
#define __Trie_Data_Structure__HashTrie__

#include "HashTableEntry.h"
#include "BitString.h"
#include "Trie.h"

static const int PRE_LEN = 32;
static const int STRIDE = 4;

template <class T>
class HashTrie : public Trie<T> {
public:
    // Constructor
    HashTrie(T iDefaultAction, int iDefaultActionSize, std::hash<BitString> *iHashFunc = 0);
    HashTrie(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, T iDefaultAction, int iDefaultActionSize, std::hash<BitString> *iHashFunc = 0);

    // Destructor
    ~HashTrie();

    // Update
    virtual void update(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, typename Trie<T>::Action iAction);

    // Lookup
    T exactPrefixMatch(BitString iKey) const;
    T longestPrefixMatch(BitString iKey) const;

    // Insert
    void insert(BitString iKey, T iAction, int iActionSize);

    // Remove
    void remove(BitString iKey);

private:
    HashTableEntry<T> *mRoot;
    std::hash<BitString> *mHashFunction;
    T mDefaultAction;

    unsigned int extract(unsigned int iKey, int iPos, int iNum) const;
    unsigned int createMask(unsigned int a, unsigned int b) const;

    T exactPrefixMatch(BitString iKey, unsigned int iHashKey, HashTableEntry<T> *iEntry, int iPos) const;
    void insert(BitString iKey, T iAction, int iActionSize, unsigned int iHashKey, HashTableEntry<T> *iEntry, int iPos);
    void remove(BitString iKey, unsigned int iHashKey, HashTableEntry<T> *iEntry, int iPos);
    int popcount(unsigned int iBitstring) const;
    string DecimalToBinaryString(int iDecimal, const int iNumOfBits) const;


};

//
//  HashTrie.cpp
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-03-24.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

//#include "HashTrie.h"
#include <bitset>
#include <iostream>
#include <cmath>

/// ==========================
//
//  Constructors
//
/// ==========================

template <class T>
HashTrie<T>::HashTrie(T iDefaultAction, int iDefaultActionSize, std::hash<BitString> *iHashFunc) : mHashFunction(iHashFunc), mDefaultAction(iDefaultAction) {
    mRoot = new HashTableEntry<T>[(int)pow(2.0, STRIDE)]();
    if (!mHashFunction) {
        mHashFunction = new std::hash<BitString>();
    }
}

template <class T>
HashTrie<T>::HashTrie(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, T iDefaultAction, int iDefaultActionSize, std::hash<BitString> *iHashFunc) : mHashFunction(iHashFunc), mDefaultAction(iDefaultAction) {
    mRoot = new HashTableEntry<T>[(int)pow(2.0, STRIDE)]();
    if (!mHashFunction) {
        mHashFunction = new std::hash<BitString>();
    }
    for (int i = 0; i < iRoutingTableSize; i++) {
        insert(iRoutingTable[i].getData(), iRoutingTable[i].getAction(), iRoutingTable[i].getActionSize());
    }
}

/// ==========================
//
//  Destructor
//
/// ==========================

template <class T>
HashTrie<T>::~HashTrie() {
/*    if (mRoot) {
        delete [] mRoot;
        mRoot = 0;
    }
    if (mHashFunction) {
        delete mHashFunction;
        mHashFunction = 0;
    }*/
}

/// ==============================
//
//  Update
//
/// ==============================

template <class T>
void HashTrie<T>::update(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, typename Trie<T>::Action iAction) {
    if (iAction == Trie<T>::Add) {
        for (int i = 0; i < iRoutingTableSize; i++) {
            insert(iRoutingTable[i].getData(), iRoutingTable[i].getAction(), iRoutingTable[i].getActionSize());
        }
    } else if (iAction == Trie<T>::Remove) {
        for (int i = 0; i < iRoutingTableSize; i++) {
            remove(iRoutingTable[i].getData());
        }
    } else {
        if (mRoot) {
            delete [] mRoot;
            mRoot = new HashTableEntry<T>[(int)pow(2.0, STRIDE)]();
        }
        for (int i = 0; i < iRoutingTableSize; i++) {
            insert(iRoutingTable[i].getData(), iRoutingTable[i].getAction(), iRoutingTable[i].getActionSize());
        }
    }
}
/// ==============================
//
//  Exact Prefix Match Lookup
//
/// ==============================

template <class T>
T HashTrie<T>::exactPrefixMatch(BitString iKey) const {
    // Hash the key
    unsigned int wHashKey = (unsigned int) (*mHashFunction)(iKey);
    // Extract first 5 bits of the key
    int wIndex = extract(wHashKey, 0, STRIDE);

    // Find corresponding entry in the root hash table
    HashTableEntry<T> *wEntry = &mRoot[wIndex];
    return exactPrefixMatch(iKey, wHashKey, wEntry, STRIDE);
}

template <class T>
T HashTrie<T>::exactPrefixMatch(BitString iKey, unsigned int iHashKey, HashTableEntry<T> *iEntry, int iPos) const {
    if (iPos >= PRE_LEN) {
        iHashKey = (unsigned int)(*mHashFunction)(BitString::intToBitString(iHashKey, PRE_LEN));
        iPos = 0;
    }
    if (iEntry->getFlag()) {
        if (BitString::intToBitString(iEntry->getKeyBitmap(), iEntry->getKeyLength()) == iKey) {
            return iEntry->getValue();
        } else {
            if (iPos == 0) {
                HashTableEntry<T> *wSubtrie = iEntry->getPtr();
                int wIndex = extract(iHashKey, iPos, STRIDE);
                if (extract(iEntry->getKeyBitmap(), wIndex, 1) == 0) {
                    return mDefaultAction;
                } else {
                    return exactPrefixMatch(iKey, iHashKey, &wSubtrie[popcount(extract(iEntry->getKeyBitmap(), 0, wIndex + 1)) - 1], iPos + STRIDE);
                }
            } else {
                return mDefaultAction;
            }
        }
    } else {
        if (iEntry->getKeyBitmap() == 0) {
            return mDefaultAction;
        } else {
            HashTableEntry<T> *wSubtrie = iEntry->getPtr();
            int wIndex = extract(iHashKey, iPos, STRIDE);
            if (extract(iEntry->getKeyBitmap(), wIndex, 1) == 0) {
                return mDefaultAction;
            } else {
                return exactPrefixMatch(iKey, iHashKey, &wSubtrie[popcount(extract(iEntry->getKeyBitmap(), 0, wIndex + 1)) - 1], iPos + STRIDE);
            }
        }
    }
}

/// ===============================
//
//  Longest Prefix Match Lookup
//
/// ===============================

template <class T>
T HashTrie<T>::longestPrefixMatch(BitString iPrefix) const {
    unsigned int wSize = (unsigned int)iPrefix.size();
    T wResult = exactPrefixMatch(iPrefix);
    if (wResult != mDefaultAction) {
        return wResult;
    } else {
        for (int i = 0; i < wSize; i++) {
            iPrefix.pop_back();
            wResult = exactPrefixMatch(iPrefix);
            if (wResult != mDefaultAction) {
                return wResult;
            }
        }
    }
    return mDefaultAction;
}

/// ==========================
//
//  Insert
//
/// ==========================

template <class T>
void HashTrie<T>::insert(BitString iKey, T iAction, int iActionSize) {
    // Hash the key
    unsigned int wHashKey = (unsigned int)(*mHashFunction)(iKey);
    // Extract first 5 bits of the key
    int wIndex = extract(wHashKey, 0, STRIDE);

    HashTableEntry<T> *wEntry = &mRoot[wIndex];
    insert(iKey, iAction, iActionSize, wHashKey, wEntry, STRIDE);
}

template <class T>
void HashTrie<T>::insert(BitString iKey, T iAction, int iActionSize, unsigned int iHashKey, HashTableEntry<T> *iEntry, int iPos) {
    if (iPos >= PRE_LEN) {
        iHashKey = (unsigned int)(*mHashFunction)(BitString::intToBitString(iHashKey, PRE_LEN));
        iPos = 0;
    }
    if (iEntry->getFlag()) {
        if (BitString::intToBitString(iEntry->getKeyBitmap(), iEntry->getKeyLength()) == iKey) {
            iEntry->setValue(iAction, iActionSize);
            return;
        } else {
            HashTableEntry<T> wCurrentEntry(iEntry->getFlag(), iEntry->getKeyBitmap(), iEntry->getKeyLength(), iEntry->getValue(), iEntry->getPtr());
            iEntry->setFlag(0);
            unsigned int wBitmap = 0;
            int wCurrentEntryIndex = extract((unsigned int)(*mHashFunction)(BitString::intToBitString(iEntry->getKeyBitmap(), iEntry->getKeyLength())), iPos, STRIDE);
            int wIndex = extract(iHashKey, iPos, STRIDE);
            if (wIndex == wCurrentEntryIndex) {
                wBitmap |= (1 << (PRE_LEN - 1 - wIndex));
                iEntry->setKeyBitmap(wBitmap);
                HashTableEntry<T> *wNewSubTrie = new HashTableEntry<T>(wCurrentEntry);
                iEntry->setPtr(wNewSubTrie, 1);
                insert(iKey, iAction, iActionSize, iHashKey, wNewSubTrie, iPos + STRIDE);
            } else {
                wBitmap |= 1 << (PRE_LEN - 1 - wCurrentEntryIndex);
                wBitmap |= 1 << (PRE_LEN - 1 - wIndex);
                iEntry->setKeyBitmap(wBitmap);
                HashTableEntry<T> *wNewSubtrie = new HashTableEntry<T>[2]();
                if (wCurrentEntryIndex < wIndex) {
                    wNewSubtrie[0] = wCurrentEntry;
                    wNewSubtrie[1].setFlag(1);
                    wNewSubtrie[1].setKeyBitmap(iKey.toInt());
                    wNewSubtrie[1].setKeyLength((int)iKey.size());
                    wNewSubtrie[1].setValue(iAction, iActionSize);
                    wNewSubtrie[1].setPtr(0, iActionSize);
                } else {
                    wNewSubtrie[1] = wCurrentEntry;
                    wNewSubtrie[0].setFlag(1);
                    wNewSubtrie[0].setKeyBitmap(iKey.toInt());
                    wNewSubtrie[0].setKeyLength((int)iKey.size());
                    wNewSubtrie[0].setValue(iAction, iActionSize);
                    wNewSubtrie[0].setPtr(0, iActionSize);
                }
                iEntry->setPtr(wNewSubtrie, 1);
            }
        }
    } else {
        if (iEntry->getKeyBitmap() == 0) {
            iEntry->setFlag(1);
            iEntry->setKeyBitmap(iKey.toInt());
            iEntry->setKeyLength(iKey.size());
            iEntry->setValue(iAction, iActionSize);
            iEntry->setPtr(0, iActionSize);
        } else {
            HashTableEntry<T> *wSubtrie = iEntry->getPtr();
            int wIndex = extract(iHashKey, iPos, STRIDE);
            if (extract(iEntry->getKeyBitmap(), wIndex, 1) == 0) {
                int wArraySize = popcount(iEntry->getKeyBitmap());
                HashTableEntry<T> *newSubTrie = new HashTableEntry<T>[wArraySize + 1]();
                int wMask = 0;
                wMask |= 1 << (PRE_LEN - 1 - wIndex);
                iEntry->setKeyBitmap(iEntry->getKeyBitmap() | wMask); // Update Bitmap
                int wArrayPosition = popcount(extract(iEntry->getKeyBitmap(), 0, wIndex + 1)) - 1;
                int j = 0;
                for (int i = 0; i < wArraySize + 1; i++) {
                    if (i == wArrayPosition) {
                        newSubTrie[i].setFlag(1);
                        newSubTrie[i].setKeyBitmap(iKey.toInt());
                        newSubTrie[i].setKeyLength((int)iKey.size());
                        newSubTrie[i].setValue(iAction, iActionSize);
                        newSubTrie[i].setPtr(0, iActionSize);
                    } else {
                        newSubTrie[i].setFlag(wSubtrie[j].getFlag());
                        wSubtrie[j].setFlag(0);
                        newSubTrie[i].setKeyBitmap(wSubtrie[j].getKeyBitmap());
                        newSubTrie[i].setKeyLength(wSubtrie[j].getKeyLength());
                        newSubTrie[i].setValue(wSubtrie[j].getValue(), wSubtrie[j].getValPtrSize());
                        newSubTrie[i].setPtr(wSubtrie[j].getPtr(), wSubtrie[j].getValPtrSize());
                        wSubtrie[j].setPtr(0, 0);
                        j++;
                    }
                }
                iEntry->setPtr(newSubTrie, 1);
                // MEMORY LEAK!?
                delete [] wSubtrie;
            } else {
                insert(iKey, iAction, iActionSize, iHashKey, &wSubtrie[popcount(extract(iEntry->getKeyBitmap(), 0, wIndex + 1)) - 1], iPos + STRIDE);
            }
        }
    }
}

/// ==========================
//
//  Remove
//
/// ==========================

template <class T>
void HashTrie<T>::remove(BitString iKey) {
    // Hash the key
    unsigned int wHashKey = (unsigned int)(*mHashFunction)(iKey);
    // Extract first 5 bits of the key
    int wIndex = extract(wHashKey, 0, STRIDE);

    HashTableEntry<T> *wEntry = &mRoot[wIndex];
    remove(iKey, wHashKey, wEntry, STRIDE);
}

template <class T>
void HashTrie<T>::remove(BitString iKey, unsigned int iHashKey, HashTableEntry<T> *iEntry, int iPos) {
    if (iPos >= PRE_LEN) {
        iHashKey = (unsigned int)(*mHashFunction)(BitString::intToBitString(iHashKey, PRE_LEN));
        iPos = 0;
    }
    if (iEntry->getFlag()) {
        if (BitString::intToBitString(iEntry->getKeyBitmap(), iEntry->getKeyLength()) == iKey) {
            iEntry->setFlag(0);
            iEntry->setKeyBitmap(0);
            iEntry->setKeyLength(0);
            iEntry->setValue(mDefaultAction, 0);
        } else {
            return;
        }
    } else {
        if (iEntry->getKeyBitmap() == 0) {
            return;
        } else {
            HashTableEntry<T> *wSubtrie = iEntry->getPtr();
            int wIndex = extract(iHashKey, iPos, STRIDE);
            if (extract(iEntry->getKeyBitmap(), wIndex, 1) == 0) {
                return;
            } else {
                int wArrayPosition = popcount(extract(iEntry->getKeyBitmap(), 0, wIndex + 1)) - 1;
                HashTableEntry<T> *wNewEntry = &wSubtrie[wArrayPosition];
                int wArraySize = popcount(iEntry->getKeyBitmap());
                if (wNewEntry->getFlag()) {
                    int wOtherPosition;
                    if (wArrayPosition == 0) {
                        wOtherPosition = 1;
                    } else {
                        wOtherPosition = 0;
                    }
                    if (wArraySize == 2 && wSubtrie[wOtherPosition].getFlag() == 1) {
                        if (wArrayPosition == 0) {
                            iEntry->setFlag(wSubtrie[1].getFlag());
                            iEntry->setKeyBitmap(wSubtrie[1].getKeyBitmap());
                            iEntry->setKeyLength(wSubtrie[1].getKeyLength());
                            iEntry->setValue(wSubtrie[1].getValue(), wSubtrie[1].getValPtrSize());
                            iEntry->setPtr(wSubtrie[1].getPtr(), wSubtrie[1].getValPtrSize());
                        } else {
                            iEntry->setFlag(wSubtrie[0].getFlag());
                            iEntry->setKeyBitmap(wSubtrie[0].getKeyBitmap());
                            iEntry->setKeyLength(wSubtrie[0].getKeyLength());
                            iEntry->setValue(wSubtrie[0].getValue(), wSubtrie[0].getValPtrSize());
                            iEntry->setPtr(wSubtrie[0].getPtr(), wSubtrie[0].getValPtrSize());
                        }
                    } else {
                        unsigned int wMask = 0;
                        wMask |= 1 << (PRE_LEN - 1 - wIndex);
                        iEntry->setKeyBitmap(iEntry->getKeyBitmap() ^ wMask); // Update Bitmap
                        HashTableEntry<T> *wNewSubtrie = new HashTableEntry<T>[wArraySize - 1]();
                        int j = 0;
                        for (int i = 0; i < wArraySize; i++) {
                            if (i != wArrayPosition) {
                                wNewSubtrie[j].setFlag(wSubtrie[i].getFlag());
                                wSubtrie[i].setFlag(0);
                                wNewSubtrie[j].setKeyBitmap((wSubtrie[i].getKeyBitmap()));
                                wNewSubtrie[j].setKeyLength(wSubtrie[i].getKeyLength());
                                wNewSubtrie[j].setValue(wSubtrie[i].getValue(), wSubtrie[i].getValPtrSize());
                                wNewSubtrie[j].setPtr(wSubtrie[i].getPtr(), wSubtrie[i].getValPtrSize());
                                wSubtrie[i].setPtr(0, 0);
                                j++;
                            }
                        }
                        iEntry->setPtr(wNewSubtrie, 1);
                        delete [] wSubtrie;
                    }
                } else {
                    remove(iKey, iHashKey, wNewEntry, iPos + STRIDE);
                }
            }
        }
    }
}

/// ==========================================================================
//
//  Extract
//
//  Returns iNum number of bits from iKey starting at position iPos
//
/// ==========================================================================

template <class T>
unsigned int HashTrie<T>::extract(unsigned int iKey, int iPos, int iNum) const {
    unsigned int wMask = createMask(iPos, iPos + iNum);
    return ((iKey & wMask) >> (PRE_LEN - iNum - iPos));
}

template <class T>
unsigned int HashTrie<T>::createMask(unsigned int a, unsigned int b) const {
    unsigned int r = 0;
    for (unsigned int i = 0; i < b - a; i++)
        r |= 1 << i;
    r = r << (PRE_LEN - a - (b - a));
    return r;
}

/// ==================================================
//
//  Popcount
//
//  Returns number of bits that are 1 in iBitstring
//
/// ==================================================

template <class T>
int HashTrie<T>::popcount(unsigned int iBitstring) const {
    std::bitset<PRE_LEN> wBitset(iBitstring);
    return (int)wBitset.count();
}

/// ======================================
//
//  Converts Decimal to Binary String
//
/// ======================================

template <class T>
string HashTrie<T>::DecimalToBinaryString(int iDecimal, const int iNumOfBits) const {
    string wString = std::bitset<32>(iDecimal).to_string();
    string wFinalString = wString.substr(wString.size() - iNumOfBits);
    return wFinalString;
}

#endif /* defined(__Trie_Data_Structure__HashTrie__) */
