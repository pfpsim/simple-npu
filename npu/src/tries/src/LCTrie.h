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
//  LCTrie.h
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-02-08.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#ifndef __Trie_Data_Structure__LCTrie__
#define __Trie_Data_Structure__LCTrie__

#include "BaseVectorEntry.h"
#include "PrefixVectorEntry.h"
#include "Trie.h"
#include "LCNode.h"
#include "BitString.h"
#include <vector>

using namespace std;

static int ADR_SIZE = 32; // bits in an address
static int FIRST_SKIP_BIT = 27;
static int FIRST_ADR_BIT = 22;
static int NO_PREFIX = -1; // empty prefix pointer

template <class T>
class LCTrie : public Trie<T> {
public:

    // Constructor
    LCTrie(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, T iDefaultAction, int iDefaultActionSize, double iFillFactor, int iRootBranchFactor);
    LCTrie(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, T iDefaultAction, int iDefaultActionSize);

    // Destructor
    virtual ~LCTrie();

    // Update
    virtual void update(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, typename Trie<T>::Action iAction);

    // Lookup
    T exactPrefixMatch(BitString iPrefix) const;
    T longestPrefixMatch(BitString iPrefix) const;

    // Print Trie
    void printTrie() const;

private:
    LCNode *mTrie;                          // Main LC-Trie Structure
    int mTrieSize;                          // Size of LC-Trie
    BaseVectorEntry *mBaseVector;           // Base Vector Array
    int mBaseVectorSize;                    // Size of Base Vector
    PrefixVectorEntry *mPrefixVector;       // Prefix Vector Array
    int mPrefixVectorSize;                  // Size of Prefix Vector
    T *mActionTable;                        // Action Table Array
    int mNumActionTableElements;            // Number of Elements in Action Table
    int mActionTableSize;                    // Size of Action Table in Bytes (for TLM Memory)
    double mFillFactor;                     // LC-Trie Fill Factor
    int mRootBranchingFactor;               // Branching Factor of Root
    int mNextFree;                          // Index of Next Free Element in LC-Trie
    T mDefaultAction;                       // Default Action

    void constructTrie(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, double iFillFactor, int iRootBranchFactor);
    void build(int iPrefix, int iFirst, int iNum, int iPos, LCNode *iTrie);
    void buildActionTable(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize);
    int binarySearch(T x, T *v, int size);
    int linearSearch(T x, T *v, int size);
    int computeNewPrefix(int iPrefix, int iFirst, int iNum);
    int computeBranch(int iPrefix, int iFirst, int iNum);
    int bestPrefixMatch(int iPrefix, int iPos, BitString iBitPat, int iBranch);
    void traverse(LCNode *iRoot, int iDepth, int *iDepths);
    int extract(int iPos, int iBranch, BitString iString) const;

};

//
//  LCTrie.cpp
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-02-08.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

//#include "LCTrie.h"
#include <cmath>

/// ======================================
//
//  Constructors
//
/// ======================================

template <class T>
LCTrie<T>::LCTrie(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, T iDefaultAction, int iDefaultActionSize, double iFillFactor, int iRootBranchFactor) : mDefaultAction(iDefaultAction) {
    constructTrie(iRoutingTable, iRoutingTableSize, iFillFactor, iRootBranchFactor);
}

template <class T>
LCTrie<T>::LCTrie(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, T iDefaultAction, int iDefaultActionSize) : LCTrie(iRoutingTable, iRoutingTableSize, iDefaultAction, iDefaultActionSize, 1.0, 0) {}

template <class T>
void LCTrie<T>::constructTrie(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, double iFillFactor, int iRootBranchFactor) {

    if (iFillFactor < 0.1 || iFillFactor > 1.0 || iRootBranchFactor < 0 || iRootBranchFactor > 20) {
        // throw exception
    }

    int wSize; // Size of entry array after duplicate removal

    mFillFactor = iFillFactor;
    mRootBranchingFactor = iRootBranchFactor;

    buildActionTable(iRoutingTable, iRoutingTableSize);

    std::sort(iRoutingTable, iRoutingTable + iRoutingTableSize, RoutingTableEntry<T>::compareEntries);

    // Remove duplicates
    wSize = iRoutingTableSize > 0 ? 1 : 0;
    for (int i = 1; i < iRoutingTableSize; i++) {
        if ((iRoutingTable[i-1].getData()).compare(iRoutingTable[i].getData())) {
            iRoutingTable[wSize++] = iRoutingTable[i];
        }
    }

    /*for (int i = 0; i < iRoutingTableSize; i++) {
     cout << "Prefix: " << iRoutingTable[i].getData() << ", Next Hop: " << iRoutingTable[i].getAction() << endl;
     }*/

    // We first build a big data structure and afterwards,
    // when we know the size, we store it in a more compact format.
    // The number of internal nodes in the tree can't be larger
    // than the number of entries and the number of leaves can
    // be at most size/fillFactor. If the branching is fixed at
    // the root there might be more leaves.
    LCNode *wLCTrie = new LCNode[wSize + (int) (wSize/mFillFactor) + (1<<mRootBranchingFactor)];
    BaseVectorEntry *wBaseVector = new BaseVectorEntry[wSize]();
    PrefixVectorEntry *wPrefixVector = new PrefixVectorEntry[wSize]();

    // Initialize a temporary array of prefix pointers
    int *wPreTemp = new int[wSize];
    for (int i = 0; i < wSize; i++) {
        wPreTemp[i] = NO_PREFIX;
    }

    // Go through the entries and put the prefixes in prefix vector
    // and the rest of the strings in base vector
    int wNBases = 0; // This many entries ended up in the base vector
    int wNPrefs = 0; // and this many in the prefix vector.
    for (int i = 0; i < wSize; i++) {
        if (i < wSize-1 && iRoutingTable[i].isPrefixOf(iRoutingTable[i+1])) {
            PrefixVectorEntry *wPTemp = new PrefixVectorEntry(iRoutingTable[i].getLength(), wPreTemp[i]);
            // Update preTemp for all entries that have this prefix
            for (int j = i + 1; j < wSize && iRoutingTable[i].isPrefixOf(iRoutingTable[j]); j++) {
                wPreTemp[j] = wNPrefs;
            }
            //wPTemp->setActionPtr(binarySearch(iRoutingTable[i].getAction(), mActionTable, mNumActionTableElements)); // Removed when templating
            wPTemp->setActionPtr(linearSearch(iRoutingTable[i].getAction(), mActionTable, mNumActionTableElements));   // Added for templated next hop
            wPrefixVector[wNPrefs++] = *wPTemp;
            delete wPTemp;
        } else {
            BaseVectorEntry *wBTemp = new BaseVectorEntry(iRoutingTable[i].getData(), iRoutingTable[i].getLength(), wPreTemp[i]);
            //wBTemp->setActionPtr(binarySearch(iRoutingTable[i].getAction(), mActionTable, mNumActionTableElements));     // Removed when templating
            wBTemp->setActionPtr(linearSearch(iRoutingTable[i].getAction(), mActionTable, mNumActionTableElements));       // Added for templated next hop
            wBaseVector[wNBases++] = *wBTemp;
            delete wBTemp;
        }
    }

    // At this point we know how much memory to allocate
    // for the base vector and the prefix vector.
    mBaseVector = new BaseVectorEntry[wNBases];
    mBaseVectorSize = wNBases;
    mPrefixVector = new PrefixVectorEntry[wNPrefs];
    mPrefixVectorSize = wNPrefs;
    //cout << "Base Vector: " << endl;
    for (int i = 0; i < wNBases; i++) {
        mBaseVector[i] = wBaseVector[i];
        //cout << mBaseVector[i].getPrefix() << ", " << mBaseVector[i].getPrefixTablePtr() << ", " << mBaseVector[i].getActionPtr() << endl;
    }
    //cout << "Prefix Vector: " << endl;
    for (int i = 0; i < wNPrefs; i++) {
        mPrefixVector[i] = wPrefixVector[i];
        //cout << mPrefixVector[i].getLength() << ", " << mPrefixVector[i].getPrefixPtr() << ", " << mPrefixVector[i].getActionPtr() << endl;
    }

    // Build the trie structure
    mNextFree = 1;
    build(0, 0, wNBases, 0, wLCTrie);

    // Allocate memory and move the tree into its final place.
    mTrie = new LCNode[mNextFree];
    for (int i = 0; i < mNextFree; i++) {
        mTrie[i].setBranchFactor(wLCTrie[i].getBranchFactor());
        mTrie[i].setSkip(wLCTrie[i].getSkip());
        mTrie[i].setLeftNodePos(wLCTrie[i].getLeftNodePos());
    }
    mTrieSize = mNextFree;
}

/// ======================================
//
//  Destructor
//
/// ======================================

template <class T>
LCTrie<T>::~LCTrie() {
    if (mTrie) {
        delete [] mTrie;
        mTrie = 0;
    }
    if (mBaseVector) {
        delete [] mBaseVector;
        mBaseVector = 0;
    }
    if (mPrefixVector) {
        delete [] mPrefixVector;
        mPrefixVector = 0;
    }
    if (mActionTable) {
        delete [] mActionTable;
        mActionTable = 0;
    }
}

/// ======================================
//
//  Build LC-Trie from Routing Table
//
/// ======================================

template <class T>
void LCTrie<T>::build(int iPrefix, int iFirst, int iNum, int iPos, LCNode *iTrie) {
    int wBranch;
    int wNewPrefix;
    int wAdr;

    // only one element, this is a leaf
    if (iNum == 1) {
        iTrie[iPos].setBranchFactor(0);
        iTrie[iPos].setSkip((int)mBaseVector[iFirst].getPrefix().length() - iPrefix);
        iTrie[iPos].setLeftNodePos(iFirst); // branch and skip are 0
        return;
    }

    // compute skip value
    wNewPrefix = computeNewPrefix(iPrefix, iFirst, iNum);

    // compute branching
    if (mRootBranchingFactor > 0 && iPrefix == 0 && iFirst == 0) {
        wBranch = mRootBranchingFactor; // fixed branching at root
    } else {
        wBranch = computeBranch(wNewPrefix, iFirst, iNum);
    }

    // address of leftmost child
    wAdr = mNextFree;
    iTrie[iPos].setBranchFactor(wBranch);
    iTrie[iPos].setSkip(wNewPrefix - iPrefix);
    iTrie[iPos].setLeftNodePos(wAdr);

    mNextFree += pow(2.0, (double)wBranch);

    // Build the subtrees
    int p = iFirst; // position in base array
    for (int bitpat = 0; bitpat < pow(2.0, (double)wBranch); bitpat++) {
        int k = 0; // count #entries matching this bit pattern
        while (p + k < iFirst + iNum) {
            if (mBaseVector[p + k].getPrefix().substr(wNewPrefix, wBranch) != "") {
                if (mBaseVector[p + k].getLength() < wBranch + wNewPrefix) {
                    // Expand Prefix
                    int wNumOfExpansions = (int)pow(2.0, (double)wBranch + wNewPrefix - mBaseVector[p + k].getLength());
                    BaseVectorEntry *wNewBaseVector = new BaseVectorEntry[mBaseVectorSize + wNumOfExpansions]();
                    for (int i = 0; i < p + k; i++) {
                        wNewBaseVector[i] = mBaseVector[i];
                    }
                    for (int i = 0; i < wNumOfExpansions; i++) {
                        BitString wExpandedPrefix = mBaseVector[p + k].getPrefix() + BitString::intToBitString(i, wBranch + wNewPrefix - mBaseVector[p + k].getLength());
                        wNewBaseVector[p + k + i].setPrefix(wExpandedPrefix);
                        wNewBaseVector[p + k + i].setLength((int)wExpandedPrefix.size());
                        wNewBaseVector[p + k + i].setActionPtr(mBaseVector[p + k].getActionPtr());
                        wNewBaseVector[p + k + i].setPrefixTablePtr(mBaseVector[p + k].getPrefixTablePtr());
                    }
                    int j = p + k + wNumOfExpansions;
                    for (int i = p + k + 1; i < mBaseVectorSize; i++) {
                        wNewBaseVector[j] = mBaseVector[i];
                        j++;
                    }
                    iNum += wNumOfExpansions - 1;
                    mBaseVectorSize += wNumOfExpansions - 1;
                    delete [] mBaseVector;
                    mBaseVector = wNewBaseVector;
                }
                if (mBaseVector[p + k].getPrefix().substr(wNewPrefix, wBranch) == BitString::intToBitString(bitpat, wBranch)) {
                    k++;
                } else {
                    break;
                }
            } else {
                break;
            }
        }

        if (k == 0) {
            // The leaf should have a pointer either to p-1 or p,
            // whichever has the longest matching prefix
            int match1 = 0, match2 = 0;
            if (p > iFirst) {
                match1 = bestPrefixMatch(wNewPrefix, p - 1, (BitString::intToBitString(bitpat, ADR_SIZE)), wBranch);
            }
            if (p < iFirst + iNum) {
                match2 = bestPrefixMatch(wNewPrefix, p, (BitString::intToBitString(bitpat, ADR_SIZE)), wBranch);
            }

            if ((match1 > match2 && p > iFirst) || p == iFirst + iNum) {
                build(wNewPrefix + wBranch, p - 1, 1, wAdr + bitpat, iTrie);
            } else {
                build(wNewPrefix + wBranch, p, 1, wAdr + bitpat, iTrie);
            }

        } else if (k == 1 && mBaseVector[p].getLength() - wNewPrefix < wBranch) {
            int bits = wBranch - mBaseVector[p].getLength() + wNewPrefix;
            for (int i = bitpat; i < bitpat + (1<<bits); i++)
                build(wNewPrefix + wBranch, p, 1, wAdr + i, iTrie);
            bitpat += (1<<bits) - 1;
        } else {
            int wTempBaseVectorSize = mBaseVectorSize;
            build(wNewPrefix + wBranch, p, k, wAdr + bitpat, iTrie);
            if (mBaseVectorSize != wTempBaseVectorSize) {
                iNum += mBaseVectorSize - wTempBaseVectorSize;
                p += mBaseVectorSize - wTempBaseVectorSize;
            }
        }

        p += k;
    }
}

/// ======================================
//
//  Compute New Prefix Skip value
//
/// ======================================

template <class T>
int LCTrie<T>::computeNewPrefix(int iPrefix, int iFirst, int iNum) {
    BitString wLow = mBaseVector[iFirst].getPrefix();
    BitString wHigh = mBaseVector[iFirst + iNum - 1].getPrefix();
    if (iPrefix > wLow.size() || iPrefix > wHigh.size()) {
        return iPrefix;
    }
    while (extract(iPrefix, 1, wLow) == extract(iPrefix, 1, wHigh)) {
        iPrefix++;
        if (iPrefix > wLow.size() || iPrefix > wHigh.size()) {
            break;
        }
    }
    return iPrefix;
}

/// ======================================
//
//  Compute Branching Factor
//
/// ======================================

template <class T>
int LCTrie<T>::computeBranch(int iPrefix, int iFirst, int iNum) {
    // Always use branching factor 2 for two elements
    if (iNum == 2)
        return 1;

    // Compute the number of bits b that can be used for branching.
    // We have at least two branches. Therefore we start the search
    // at b = 2, i.e. 2^b = 4 branches.
    int b = 1;
    int wCount = 0;
    do {
        b++;
        if (iNum < mFillFactor*pow(2.0, (double)b) ||
            iPrefix + b > ADR_SIZE)
            break;
        int i = iFirst;
        wCount = 0;
        for (int wPat = 0; wPat < pow(2.0, (double)b); wPat++) {
            bool wPatFound = false;
            if (i < iFirst + iNum && mBaseVector[i].getLength() < (iPrefix + b)) {
                i++;
                continue;
            }
            while (i < iFirst + iNum && wPat == extract(iPrefix, b, mBaseVector[i].getPrefix())) {
                if (mBaseVector[i].getLength() >= (iPrefix + b)) {
                    wPatFound = true;
                }
                i++;
            }
            if (wPatFound) {
                wCount++;
            }
        }
    } while (wCount >= mFillFactor*pow(2.0, (double)b));
    return b - 1;
}

/// ======================================
//
//  Build Next Hop Table
//
/// ======================================

template <class T>
void LCTrie<T>::buildActionTable(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize) {
    // Extract the nexthop addresses from the entry array and get max size
    T *wNextTemp = new T[iRoutingTableSize];
    int wSize = 0;
    for (int i = 0; i < iRoutingTableSize; i++) {
        wNextTemp[i] = iRoutingTable[i].getAction();
        wSize += iRoutingTable[i].getActionSize();
    }

    // Sort it
    std::sort(wNextTemp, wNextTemp + iRoutingTableSize, std::less<T>());

    // Remove duplicates and save them in a vector
    int wCount = iRoutingTableSize > 0 ? 1 : 0;
    vector<T> wDuplicates;
    for (int i = 1; i < iRoutingTableSize; i++) {
        if (wNextTemp[i-1] != wNextTemp[i]) {
            wNextTemp[wCount++] = wNextTemp[i];
        } else {
            wDuplicates.push_back(wNextTemp[i]);
        }
    }

    // Find size of duplicates and remove from size total
    for (int i = 0; i < wDuplicates.size(); i++) {
        for (int j = 0; j < iRoutingTableSize; j++) {
            if (iRoutingTable[j].getAction() == wDuplicates[i]) {
                wSize -= iRoutingTable[j].getActionSize();
                break;
            }
        }
    }

    // Move the elements to an array of proper size
    T *wAction = new T[wCount];
    for (int i = 0; i < wCount; i++) {
        wAction[i] = wNextTemp[i];
    }
    mActionTable = wAction;
    mNumActionTableElements = wCount;
    mActionTableSize = wSize;
}

/// ======================================
//
//  Update
//
/// ======================================

template <class T>
void LCTrie<T>::update(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, typename Trie<T>::Action iAction) {
    if (iAction == Trie<T>::Reconstruct) {
        if (mTrie) {
            delete [] mTrie;
            mTrie = 0;
        }
        if (mBaseVector) {
            delete [] mBaseVector;
            mBaseVector = 0;
        }
        if (mPrefixVector) {
            delete [] mPrefixVector;
            mPrefixVector = 0;
        }
        if (mActionTable) {
            delete [] mActionTable;
            mActionTable = 0;
        }
        mTrieSize = 0;
        mBaseVectorSize = 0;
        mPrefixVectorSize = 0;
        mNumActionTableElements = 0;

        constructTrie(iRoutingTable, iRoutingTableSize, mFillFactor, mRootBranchingFactor);

    } else if (iAction == Trie<T>::Add) {
        throw "Unable to insert prefixes into LC-Trie. It can only be reconstructed.";
    } else {
        throw "Unable to remove prefixes from LC-Trie. It can only be reconstructed.";
    }
}

/// ======================================
//
//  Exact Prefix Match Lookup
//
/// ======================================

template <class T>
T LCTrie<T>::exactPrefixMatch(BitString iPrefix) const {
    // Traverse the trie
    LCNode *wNode = &mTrie[0];
    int wPos = wNode->getSkip();
    int wBranch = wNode->getBranchFactor();
    int wAdr = wNode->getLeftNodePos();
    while (wBranch != 0) {
        BitString wTempPrefix = iPrefix;
        if (wPos + wBranch > iPrefix.length()) {
            int wLengthDifference = wPos + wBranch - (int)iPrefix.length();
            for (int i =0; i < wLengthDifference; i++) {
                wTempPrefix += '0';
            }
        }
        wNode = &mTrie[wAdr + extract(wPos, wBranch, wTempPrefix)];
        wPos += wBranch + wNode->getSkip();
        wBranch = wNode->getBranchFactor();
        wAdr = wNode->getLeftNodePos();
    }

    // Was this a hit?
    //if(mBaseVector[wAdr].getPrefix() == iPrefix.substr(0, wPos)) {
    if(mBaseVector[wAdr].getPrefix() == iPrefix) {
        return mActionTable[mBaseVector[wAdr].getActionPtr()];
    }

    // If not, look in the prefix tree
    int wPreAdr = mBaseVector[wAdr].getPrefixTablePtr();
    while (wPreAdr != NO_PREFIX) {
        if (mBaseVector[wAdr].getPrefix().substr(0, mPrefixVector[wPreAdr].getLength()) == iPrefix && mPrefixVector[wPreAdr].getLength() == iPrefix.length()) {
            return mActionTable[mPrefixVector[wPreAdr].getActionPtr()];
        }
        wPreAdr = mPrefixVector[wPreAdr].getPrefixPtr();
    }

    return mDefaultAction; // not found
}

/// ======================================
//
//  Longest Prefix Match Lookup
//
/// ======================================

template <class T>
T LCTrie<T>::longestPrefixMatch(BitString iPrefix) const {
    // Traverse the trie
    LCNode *wNode = &mTrie[0];
    int wPos = wNode->getSkip();
    int wBranch = wNode->getBranchFactor();
    int wAdr = wNode->getLeftNodePos();
    while (wBranch != 0) {
        BitString wTempPrefix = iPrefix;
        if (wPos + wBranch > iPrefix.length()) {
            int wLengthDifference = wPos + wBranch - (int)iPrefix.length();
            for (int i =0; i < wLengthDifference; i++) {
                wTempPrefix += '0';
            }
        }
        wNode = &mTrie[wAdr + extract(wPos, wBranch, wTempPrefix)];
        wPos += wBranch + wNode->getSkip();
        wBranch = wNode->getBranchFactor();
        wAdr = wNode->getLeftNodePos();
    }

    // Was this a hit?
    if(mBaseVector[wAdr].getPrefix() == iPrefix.substr(0, wPos)) {
        return mActionTable[mBaseVector[wAdr].getActionPtr()];
    }

    // If not, look in the prefix tree
    int wPreAdr = mBaseVector[wAdr].getPrefixTablePtr();
    while (wPreAdr != NO_PREFIX) {
        if (mBaseVector[wAdr].getPrefix().substr(0, mPrefixVector[wPreAdr].getLength()) == iPrefix && mPrefixVector[wPreAdr].getLength() == iPrefix.length()) {
            return mActionTable[mPrefixVector[wPreAdr].getActionPtr()];
        }
        wPreAdr = mPrefixVector[wPreAdr].getPrefixPtr();
    }

    //Look for best match
    wPreAdr = mBaseVector[wAdr].getPrefixTablePtr();
    while (wPreAdr != NO_PREFIX) {
        if (mBaseVector[wAdr].getPrefix().substr(0, mPrefixVector[wPreAdr].getLength()) == iPrefix.substr(0, mPrefixVector[wPreAdr].getLength())) {
            return mActionTable[mPrefixVector[wPreAdr].getActionPtr()];
        }
        wPreAdr = mPrefixVector[wPreAdr].getPrefixPtr();
    }

    return mDefaultAction; // not found
}

/// ===========================================================================================
//
//  Extract
//
//  Returns the decimal value of the iBranch next bits from iString starting at iPos
//
/// ===========================================================================================

template <class T>
int LCTrie<T>::extract(int iPos, int iBranch, BitString iString) const {
    if ((iPos + iBranch) > iString.size()) {
        return 0;
    }
    BitString wString = iString.substr(iPos, iBranch);
    int temp =  wString.toInt();
    return temp;
}

/// ================================================
//
//  Find longest matching prefix in prefix table
//
/// ================================================

template <class T>
int LCTrie<T>::bestPrefixMatch(int iPrefix, int iPos, BitString iBitPat, int iBranch) {
    int wMatch = 0;
    int wPrep =  mBaseVector[iPos].getPrefixTablePtr();
    while (wPrep != NO_PREFIX && wMatch == 0) {
        int wLength = mPrefixVector[wPrep].getLength();
        if (wLength > iPrefix && extract(iPrefix, wLength - iPrefix, mBaseVector[iPos].getPrefix()) == extract(ADR_SIZE - iBranch, wLength - iPrefix, iBitPat)) {
            wMatch = wLength;
        }
        else {
            wPrep = mPrefixVector[wPrep].getPrefixPtr();
        }
    }
    return wMatch;
}

/// ======================================
//
//  Binary Search Algorithm
//
/// ======================================

template <class T>
int LCTrie<T>::binarySearch(T x, T *v, int size) {
    int low, high, mid;

    low = 0;
    high = size;
    while (low <= high) {
        mid = (low + high) / 2;
        if (stoi(x, 0, 2) < stoi(v[mid], 0, 2))
            high = mid - 1;
        else if (stoi(x, 0, 2) > stoi(v[mid], 0, 2))
            low = mid + 1;
        else
            return mid;

    }
    return -1;
}

/// ======================================
//
//  Linear Search Algorithm
//
/// ======================================

template <class T>
int LCTrie<T>::linearSearch(T x, T *v, int size) {
    for (int i = 0; i < size; i++) {
        if (x == v[i]) {
            return i;
        }
    }

    return -1;
}

/// =============================================
//
//  Traverses Trie and counts depth of leaves
//
/// =============================================

template <class T>
void LCTrie<T>::traverse(LCNode *iRoot, int iDepth, int *iDepths) {
    if (iRoot->getBranchFactor() == 0) {// leaf
        (iDepths[iDepth])++;
    } else {
        for (int i = 0; i < 1<<iRoot->getBranchFactor(); i++) {
            traverse(&mTrie[iRoot->getLeftNodePos()+i], iDepth + 1, iDepths);
        }
    }
}

/// =============================================
//
//  Print Trie
//
/// =============================================

template <class T>
void LCTrie<T>::printTrie() const {
    cout << "ActionTable:" << endl;;
    for (int i = 0;  i < mNumActionTableElements; i++)
        cout << "  " << i << " " << mActionTable[i] << endl;

    cout << "PrefixVector (length, prefix, nexthop):" << endl;
    for (int i = 0;  i < mPrefixVectorSize; i++) {
        cout << "  " << i << " " << mPrefixVector[i].getLength();
        if (mPrefixVector[i].getPrefixPtr() == NO_PREFIX) {
            cout << " E";
        } else {
            cout << " " << mPrefixVector[i].getPrefixPtr();
        }
        cout << " "  << mPrefixVector[i].getActionPtr() << endl;
    }

    cout << "BaseVector (prefix, nexthop, length):" << endl;
    for (int i = 0;  i < mBaseVectorSize; i++) {
        cout << "  " << i;
        if (mBaseVector[i].getPrefixTablePtr() == NO_PREFIX) {
            cout << " E";
        } else {
            cout << " " << mBaseVector[i].getPrefixTablePtr();
        }
        cout << " " << mBaseVector[i].getActionPtr();
        cout << " " << mBaseVector[i].getLength();
        cout << " " << mBaseVector[i].getPrefix() << endl;
    }

    cout << "Trie (branch, skip, adr):" << endl;
    for (int i = 0; i < mTrieSize; i++){
        LCNode *wNode = &mTrie[i];
        cout << "  " << i << " " << wNode->getBranchFactor();
        cout << " " << wNode->getSkip();
        cout << " " << wNode->getLeftNodePos() << endl;
    }
}

#endif /* defined(__Trie_Data_Structure__LCTrie__) */