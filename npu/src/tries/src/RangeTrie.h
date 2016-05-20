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
//  RangeTrie.h
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-05-29.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#ifndef __Trie_Data_Structure__RangeTrie__
#define __Trie_Data_Structure__RangeTrie__

#include "Trie.h"
#include "RangeTrieNode.h"
#include "PrefixTree.h"
#include "BitString.h"

static const bool VERBOSE_TRIE = false;

template <class T>
class RangeTrie : public Trie<T> {
public:
    // Constructors
    RangeTrie (RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, T iDefaultAction, int iDefaultActionSize, unsigned int iPrefixLength = 32, unsigned int iNumOfSubComparators = 3, unsigned int iMaxComparatorWidth = 32, unsigned int iMemoryBandwidth = 256);

    // Destructor
    virtual ~RangeTrie();

    // Update
    virtual void update(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, typename Trie<T>::Action iAction);

    // Insert
    void insert(BitString iPrefix, T iAction, int iActionSize);

    // Remove
    void remove(BitString iPrefix);

    // Lookups
    virtual T exactPrefixMatch(BitString iPrefix) const;
    virtual T longestPrefixMatch(BitString iPrefix) const;

private:
    RangeTrieNode *mRoot;
    unsigned int mMaxComparatorWidth;
    unsigned int mNumOfSubComparators;
    unsigned int mMemoryBandwidth;
    T *mActionTable;
    unsigned int mActionTableSize;
    unsigned int mPrefixLength;
    PrefixTree<int> *mPrefixTree;
    T mDefaultAction;

    void buildTrie(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize);
    void buildTopDown(RangeTrieNode *iNode, BitString *iInputData, int iInputDataSize, int *iRegionIDs, int iPos, int iPreviousPos, int iMaxLength);
    void buildBottomUp(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, int iPos);
    int findMaxLength(RoutingTableEntry<T> *iEntries, int iNum);
    int extract(int iPos, int iBranch, BitString iString) const;
    RangeTrieNode* getNodeForPrefix(BitString iPrefix) const;
    void changeRegionId(RangeTrieNode *iNode, int iOldId, int iNewId);

};

//
//  RangeTrie.cpp
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-05-29.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

//#include "RangeTrie.h"
#include <cmath>
#include <vector>
#include <stack>
#include <map>

int NODECOUNT = 0;

/// ============================================
//
//  Constructor
//
/// ============================================

template <class T>
RangeTrie<T>::RangeTrie(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, T iDefaultAction, int iDefaultActionSize, unsigned int iPrefixLength, unsigned int iNumOfSubComparators, unsigned int iMaxComparatorWidth, unsigned int iMemoryBandwidth) : mMaxComparatorWidth(iMaxComparatorWidth), mMemoryBandwidth(iMemoryBandwidth), mDefaultAction(iDefaultAction), mPrefixLength(iPrefixLength), mNumOfSubComparators(iNumOfSubComparators), mRoot(0), mActionTable(0), mActionTableSize(0), mPrefixTree(0) {

    // Build Range Trie
    buildTrie(iRoutingTable, iRoutingTableSize);
}

/// ============================================
//
//  Destructor
//
/// ============================================

template <class T>
RangeTrie<T>::~RangeTrie() {
    if (mRoot) {
        delete mRoot;
    }
    if (mActionTable) {
        delete [] mActionTable;
    }
}

/// ======================================================================================================
//
//  Build Range Trie
//
//  Preprocesses the routing table entries to create an equivalent set of ranges that the entries cover
//
/// ======================================================================================================

template <class T>
void RangeTrie<T>::buildTrie(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize) {
    mRoot = new RangeTrieNode();

    // Make copy of routing table
    RoutingTableEntry<T> *wRoutingTable = new RoutingTableEntry<T>[iRoutingTableSize];
    for (int i = 0; i < iRoutingTableSize; i++) {
        wRoutingTable[i] = iRoutingTable[i];
    }

    // Sort the prefixes
    std::sort(wRoutingTable, wRoutingTable + iRoutingTableSize, RoutingTableEntry<T>::compareEntries);


    if (VERBOSE_TRIE) {
        // cout << "\nPrefixes:" << endl;
        for (int i = 0; i < iRoutingTableSize; i++) {
            // cout << wRoutingTable[i].getAction() << ": " << wRoutingTable[i].getData() << "*" << endl;
        }
    }

    // Find maximum length of prefixes
    int wMaxLength = findMaxLength(wRoutingTable, iRoutingTableSize);
    if (wMaxLength <= mPrefixLength) {
        wMaxLength = mPrefixLength;
    } else {
        mPrefixLength = wMaxLength;
    }

    // Adjust all prefixes so that their length matches the maximum length (adjusted by padding with zeros). Also called mask.
    for (int i = 0; i < iRoutingTableSize; i++) {
        if (wRoutingTable[i].getLength() < wMaxLength) {
            BitString wString = wRoutingTable[i].getData();
            for (int j = 0; j < wMaxLength - wRoutingTable[i].getLength(); j++) {
                wString += '0';
            }
            wRoutingTable[i].setData(wString);
        }
    }

    if (VERBOSE_TRIE) {
        // Print masks
        // cout << "\nMASK: " << endl;
        for (int i = 0; i < iRoutingTableSize; i++) {
            // cout << wRoutingTable[i].getAction() << ": " << wRoutingTable[i].getData() << "/" <<wRoutingTable[i].getLength() << endl;
        }
    }

    // Create Intervals for each prefix
    vector<BitString> wLowerLimits;
    vector<BitString> wUpperLimits;

    BitString wMaxAddress = BitString::intToBitString(pow(2, wMaxLength), wMaxLength + 1);

    // The lower limit is always the mask itself
    // The upper limit is the first string that the prefix is not a longest prefix match of
    for (int i = 0; i < iRoutingTableSize; i++) {
        wLowerLimits.push_back(wRoutingTable[i].getData());
        BitString wTempUpperLimit = wRoutingTable[i].getData();
        int j = 0;
        for (j = wRoutingTable[i].getLength(); j > 0; j--) {
            if (wRoutingTable[i].getData()[j - 1] == 0) {
                wTempUpperLimit[j - 1] = 1;
                break;
            } else {
                wTempUpperLimit[j - 1] = 0;
            }
        }
        if (j == 0) {
            wTempUpperLimit = wMaxAddress;
        }
        wUpperLimits.push_back(wTempUpperLimit);
    }

    if (VERBOSE_TRIE) {
        // Print intervals
        // cout << "\nIntervals:" << endl;
        for (int i = 0; i < iRoutingTableSize; i++) {
            // cout << "[" << wLowerLimits[i] << " (" << wLowerLimits[i].toInt() << ")" << " - " << "(" << wUpperLimits[i].toInt() << ") " << wUpperLimits[i] << ")" << endl;
        }
    }

    // Create Regions
    BitString wTempUpperLimit = wUpperLimits[0];
    stack<BitString> wUpperLimitStack;

    vector<BitString> wNewLowerLimits;
    vector<BitString> wNewUpperLimits;

    if (wLowerLimits[0].toInt() != 0) {
        wNewLowerLimits.push_back(BitString::intToBitString(0, wMaxLength));
        wNewUpperLimits.push_back(wLowerLimits[0]);
        wNewLowerLimits.push_back(wLowerLimits[0]);
    } else {
        wNewLowerLimits.push_back(wLowerLimits[0]);
    }

    for (int i = 0; i < iRoutingTableSize - 1; i++) {
        if (wUpperLimits[i].toInt() > wLowerLimits[i + 1].toInt()) {
            if (!wUpperLimitStack.empty()) {
                while (!wUpperLimitStack.empty() && wLowerLimits[i].toInt() >= wUpperLimitStack.top().toInt()) {
                    wUpperLimitStack.pop();
                }
                if (!wUpperLimitStack.empty() && wUpperLimits[i].toInt() < wUpperLimitStack.top().toInt()) {
                    if (wUpperLimits[i] != wUpperLimitStack.top()) {
                        wUpperLimitStack.push(wUpperLimits[i]);
                    }
                } else if (wUpperLimitStack.empty()) {
                    wUpperLimitStack.push(wUpperLimits[i]);
                }
            } else {
                wUpperLimitStack.push(wUpperLimits[i]);
            }
            if (wLowerLimits[i + 1] != wNewLowerLimits[wNewLowerLimits.size() - 1]) {
                wNewUpperLimits.push_back(wLowerLimits[i + 1]);
                wNewLowerLimits.push_back(wLowerLimits[i + 1]);
            }
        } else if (wUpperLimits[i].toInt() < wLowerLimits[i + 1].toInt()) {
            wNewUpperLimits.push_back(wUpperLimits[i]);
            while (!wUpperLimitStack.empty() && wLowerLimits[i + 1].toInt() > wUpperLimitStack.top().toInt()) {
                if (wNewUpperLimits[wNewUpperLimits.size() - 1] != wUpperLimitStack.top()) {
                    wNewLowerLimits.push_back(wNewUpperLimits[wNewUpperLimits.size() - 1]);
                    wNewUpperLimits.push_back(wUpperLimitStack.top());
                }
                wUpperLimitStack.pop();
            }
            if (wNewUpperLimits[wNewUpperLimits.size() - 1] != wLowerLimits[i + 1]) {
                wNewLowerLimits.push_back(wNewUpperLimits[wNewUpperLimits.size() - 1]);
                wNewUpperLimits.push_back(wLowerLimits[i + 1]);
            }
            wNewLowerLimits.push_back(wLowerLimits[i + 1]);
        } else {
            wNewUpperLimits.push_back(wUpperLimits[i]);
            wNewLowerLimits.push_back(wLowerLimits[i + 1]);
        }
    }

    if (wUpperLimits[wUpperLimits.size() - 1].toInt() < wUpperLimitStack.top().toInt()) {
        wUpperLimitStack.push(wUpperLimits[wUpperLimits.size() - 1]);
    }

    while(!wUpperLimitStack.empty()) {
        if (wNewLowerLimits[wNewLowerLimits.size() - 1].toInt() < wUpperLimitStack.top().toInt()) {
            wNewUpperLimits.push_back(wUpperLimitStack.top());
            if (wUpperLimitStack.size() != 1) {
                wNewLowerLimits.push_back(wUpperLimitStack.top());
            }
            wUpperLimitStack.pop();
        } else {
            wUpperLimitStack.pop();
        }
    }

    if (wNewUpperLimits[wNewUpperLimits.size() - 1] != wMaxAddress) {
        wNewLowerLimits.push_back(wNewUpperLimits[wNewUpperLimits.size() - 1]);
        wNewUpperLimits.push_back(wMaxAddress);
    }

    wUpperLimits = wNewUpperLimits;
    wLowerLimits = wNewLowerLimits;

    vector<T> wActionVector;
    map<int, int> wActionMap;  //key = index in routing table, value = id
    vector<int> wIdsVector;
    int wCurrentId = 0;

    int wIndex = 0;
    for (int i = 0; i < wLowerLimits.size(); i++) {
        if (wIndex >= iRoutingTableSize) {
            wIndex = iRoutingTableSize - 1;
        }
        for (int j = wIndex; j >= 0; j--) {
            if (wLowerLimits[i].substr(0, wRoutingTable[j].getLength()) == wRoutingTable[j].getData().substr(0, wRoutingTable[j].getLength())) {
                bool wExisted = false;
                int wKey;
                if (wActionMap.find(j) == wActionMap.end()) {
                    wKey = j;
                    wActionVector.push_back(wRoutingTable[j].getAction());
                    wIdsVector.push_back(wCurrentId);
                    wActionMap.insert({wKey, wCurrentId++});
                } else {
                    wIdsVector.push_back(wActionMap.find(j)->second);
                    wExisted = true;
                }
                if (j == wIndex) {
                    wIndex++;
                    for (int k = j + 1; k < iRoutingTableSize; k++) {
                        if (wLowerLimits[i].substr(0, wRoutingTable[k].getLength()) == wRoutingTable[k].getData().substr(0, wRoutingTable[k].getLength())) {
                            if (!wExisted) {
                                wActionVector.pop_back();
                                wActionMap.erase(wKey);
                                wKey = k;
                                wActionVector.push_back(wRoutingTable[k].getAction());
                                wActionMap.insert({wKey, wCurrentId - 1});
                            } else {
                                wIdsVector.pop_back();
                                if (wActionMap.find(k) == wActionMap.end()) {
                                    wKey = k;
                                    wActionVector.push_back(wRoutingTable[k].getAction());
                                    wIdsVector.push_back(wCurrentId);
                                    wActionMap.insert({wKey, wCurrentId++});
                                } else {
                                    wIdsVector.push_back(wActionMap.find(k)->second);
                                    wExisted = true;
                                }
                            }
                            wIndex++;
                        } else {
                            break;
                        }
                    }
                }
                break;
            }
            if (j == 0) {
                if (wActionMap.find(-1) == wActionMap.end()) {
                    wActionVector.push_back(mDefaultAction);
                    wIdsVector.push_back(wCurrentId);
                    wActionMap.insert({-1, wCurrentId++});
                } else {
                    wIdsVector.push_back(wActionMap.find(-1)->second);
                }
            }
        }
    }

    // Build Prefix Tree
    mPrefixTree = new PrefixTree<int>(-1, sizeof(int));
    for (int i = 0; i < iRoutingTableSize; i++) {
        if (wActionMap.find(i) == wActionMap.end()) {
            wActionVector.push_back(wRoutingTable[i].getAction());
            wActionMap.insert({i,wCurrentId++});
        }
        mPrefixTree->insert(iRoutingTable[i].getData(), wActionMap.find(i)->second, sizeof(int));
    }


    T *wActionTable = new T[wActionVector.size()];
    int *wRegionIDs = new int[wIdsVector.size()];
    for (int i = 0; i < wActionVector.size(); i++) {
        wActionTable[i] = wActionVector[i];
        //wRegionIDs[i] = i;
    }
    for (int i = 0; i < wIdsVector.size(); i++) {
        wRegionIDs[i] = wIdsVector[i];
    }
    mActionTable = wActionTable;
    mActionTableSize = (unsigned int) wActionVector.size();

    if (VERBOSE_TRIE) {
        // Print Regions
        // cout << "\nRegions:" << endl;
        for (int i = 0; i < wUpperLimits.size(); i++) {
            // cout << "R" << i + 1 << ": [" << wLowerLimits[i] << " (" << wLowerLimits[i].toInt() << ")" << " - " << "(" << wUpperLimits[i].toInt() << ") " << wUpperLimits[i] << ")" << " - " << wRegionIDs[i] << endl;
        }

        // Print Next Hops
        // cout << "\nNext Hops:" << endl;
        for (int i = 0; i < wActionVector.size(); i++) {
            // cout << i << ": " << wActionVector[i] << endl;
        }
    }

    wLowerLimits.erase(wLowerLimits.begin());

    buildTopDown(mRoot, wLowerLimits.data(), (int)wLowerLimits.size(), wRegionIDs, 0, 0, wMaxLength);
}

/// ================================================
//
//  Top-Down Heuristic for Range Trie Construction
//
/// ================================================

template <class T>
void RangeTrie<T>::buildTopDown(RangeTrieNode *iNode, BitString *iInputData, int iInputDataSize, int *iRegionIDs, int iPos, int iPreviousPos, int iMaxLength) {
    int wMaxLength = iMaxLength;
    if (iPos != 0) {
        wMaxLength -= iPos;
        for (int i = 0; i < iInputDataSize; i++) {
            iInputData[i] = iInputData[i].substr(iPos - iPreviousPos, iMaxLength);
        }
    }

    if (iInputDataSize == 1) {
        iNode->setAddressAlignment(false);
        iNode->setCommonPrefix(-1);
        iNode->setCommonPrefixLength(0);
        iNode->setCommonSuffix(-1);
        iNode->setCommonSuffixLength(0);
        iNode->setNumOfComparisons(1);
        iNode->setComparePosition(iPos);
        iNode->setCompareLength(wMaxLength);
        iNode->setComparisons(new unsigned int(iInputData[0].toInt()));
        iNode->setRegionIDs(iRegionIDs);
        if (VERBOSE_TRIE) {
            // cout << "\nNODE " << NODECOUNT++ << ":" << endl;
            // cout << BitString::intToBitString(iNode->getComparisons()[0], wMaxLength) << endl;
        }
        return;
    }

    if (VERBOSE_TRIE) {
        // cout << "\nADDRESSES:" << endl;
        for (int i = 0; i < iInputDataSize; i++) {
            // cout << i << ". " << iInputData[i] << endl;
        }
    }

    // Common Suffix
    BitString wCommonSuffix;
    for (int i = wMaxLength - 1; i >= 0; i--) {
        bool wNoCommonSuffix = false;
        BitString wSuffix = iInputData[0].substr(i);
        for (int j = 1; j < iInputDataSize; j++) {
            if (iInputData[j].substr(i) != wSuffix) {
                wNoCommonSuffix = true;
                break;
            }
        }
        if (wNoCommonSuffix) {
            break;
        } else {
            wCommonSuffix = wSuffix;
        }
    }

    // Common Prefix
    BitString wCommonPrefix;
    for (int i = 0; i < wMaxLength; i++) {
        bool wNoCommonPrefix = false;
        BitString wPrefix = iInputData[0].substr(0, i);
        for (int j = 0; j < iInputDataSize; j++) {
            if (iInputData[j].substr(0, i) != wPrefix) {
                wNoCommonPrefix = true;
            }
        }
        if (wNoCommonPrefix) {
            break;
        } else {
            wCommonPrefix = wPrefix;
        }
    }

    //Determine Comparison Width
    int wComparisonWidth = 0;
    int wMostBranches = 0;
    int wNumPossibleComparisons = 0;
    int wNumAvailableComparisons = 0;
    int wCurrentWidth = mMaxComparatorWidth;
    for (int i = 1; i <= mNumOfSubComparators; i++) {
        wCurrentWidth = mMaxComparatorWidth/i;
        wNumPossibleComparisons = iInputDataSize/i + 1;
        wNumAvailableComparisons = mMemoryBandwidth/wCurrentWidth;
        if (wNumAvailableComparisons <= wNumPossibleComparisons) {
            if (wNumAvailableComparisons > wMostBranches) {
                wMostBranches = wNumAvailableComparisons;
                wComparisonWidth = wCurrentWidth;
            }
        } else {
            if (wNumPossibleComparisons > wMostBranches) {
                wMostBranches = wNumPossibleComparisons;
                wComparisonWidth = wCurrentWidth;
            }
        }
    }

    if (wMaxLength - wCommonPrefix.size() - wCommonSuffix.size() < wComparisonWidth) {
        wComparisonWidth = wMaxLength - (int)wCommonPrefix.size() - (int)wCommonSuffix.size();
    }

    if (VERBOSE_TRIE) {
        // cout << "\nComparison Width: " << wComparisonWidth << endl;
        // cout << "Number of Branches:" << wMostBranches << endl;
    }

    // Create groups
    vector<BitString> wGroupBounds;
    int wNumOfGroupBounds = 1;
    wGroupBounds.push_back(iInputData[0].substr(wCommonPrefix.size(), wComparisonWidth));
    for (int i = 1; i < iInputDataSize; i++) {
        BitString wSubstring = iInputData[i].substr(wCommonPrefix.size(), wComparisonWidth);
        if (wSubstring != wGroupBounds[wNumOfGroupBounds - 1]) {
            wGroupBounds.push_back(wSubstring);
            wNumOfGroupBounds++;
        }
    }

    if (VERBOSE_TRIE) {
        // cout << "\nGROUP BOUNDS:" << endl;
        for (int i = 0; i < wNumOfGroupBounds; i++) {
            // cout << i << ". " << wGroupBounds[i] << endl;
        }
    }

    // Merge necessary groups
    int wNumOfBoundsPerRegion[wNumOfGroupBounds];
    for (int i = 0; i < wNumOfGroupBounds; i++) {
        wNumOfBoundsPerRegion[i] = 0;
    }

    int j = 0;
    double wAverage = 0.0;
    for (int i = 0; i < wNumOfGroupBounds; i++) {
        for (; j < iInputDataSize;) {
            if (wGroupBounds[i] == iInputData[j].substr(wCommonPrefix.size(), wComparisonWidth)) {
                wNumOfBoundsPerRegion[i]++;
                j++;
            } else {
                wAverage += wNumOfBoundsPerRegion[i];
                break;
            }
        }
    }

    wAverage += wNumOfBoundsPerRegion[wNumOfGroupBounds - 1];

    wAverage /= wNumOfGroupBounds;

    int wIntAverage = (unsigned int)wAverage;

    if ((wAverage - (double)wIntAverage) > 0) {
        wAverage = wIntAverage + 1;
    }

    if (VERBOSE_TRIE) {
        // cout << "\nBOUNDS PER GROUP:" << endl;
        for (int i = 0; i < wNumOfGroupBounds; i++) {
            // cout << i << ". " << wNumOfBoundsPerRegion[i] << endl;
        }

        // cout << "\nAverage: " << wAverage << endl;
    }

    BitString **wGroups = new BitString*[wNumOfGroupBounds];
    int **wGroupRegionIDs = new int*[wNumOfGroupBounds];
    j = 0;
    int wNumOfGroups = 0;
    int wNumOfBoundsPerGroup[wNumOfGroupBounds];
    int wPositions[wNumOfGroupBounds];

    for (int i = 0; i < wNumOfGroupBounds; i++) {
        wGroups[i] = 0;
        wGroupRegionIDs[i] = 0;
        wNumOfBoundsPerGroup[i] = 0;
        wPositions[i] = 0;
    }

    if (wNumOfGroupBounds > wMostBranches) {
        for (int i = 0; i < wNumOfGroupBounds; i++) {
            if (wNumOfBoundsPerRegion[i] >= wAverage || i == wNumOfGroupBounds - 1) {
                wGroups[wNumOfGroups] = new BitString[wNumOfBoundsPerRegion[i]];
                wGroupRegionIDs[wNumOfGroups] = new int[wNumOfBoundsPerRegion[i] + 1];
                wNumOfBoundsPerGroup[wNumOfGroups] = wNumOfBoundsPerRegion[i];
                wPositions[wNumOfGroups] = iPos + (int)wCommonPrefix.size() + wComparisonWidth;
                int wIndex = 0;
                for (; j < iInputDataSize;) {
                    if (wGroupBounds[i] == iInputData[j].substr(wCommonPrefix.size(), wComparisonWidth)) {
                        wGroups[wNumOfGroups][wIndex] = iInputData[j];
                        wGroupRegionIDs[wNumOfGroups][wIndex] = iRegionIDs[j];
                        j++;
                        wIndex++;
                    } else {
                        break;
                    }
                }
                wGroupRegionIDs[wNumOfGroups][wIndex] = iRegionIDs[j];
            } else {
                wGroupRegionIDs[wNumOfGroups] = new int[wNumOfBoundsPerRegion[i] + wNumOfBoundsPerRegion[i + 1]];
                wNumOfBoundsPerGroup[wNumOfGroups] = wNumOfBoundsPerRegion[i] + wNumOfBoundsPerRegion[i+1];
                wPositions[wNumOfGroups] = iPos + (int)wCommonPrefix.size();
                wGroups[wNumOfGroups] = new BitString[wNumOfBoundsPerRegion[i] + wNumOfBoundsPerRegion[i+1]];
                int wIndex = 0;
                for (; j < iInputDataSize;) {
                    if (wGroupBounds[i] == iInputData[j].substr(wCommonPrefix.size(), wComparisonWidth)) {
                        wGroups[wNumOfGroups][wIndex] = iInputData[j];
                        wGroupRegionIDs[wNumOfGroups][wIndex] = iRegionIDs[j];
                        j++;
                        wIndex++;
                    } else {
                        break;
                    }
                }
                i++;
                for (; j < iInputDataSize;) {
                    if (wGroupBounds[i] == iInputData[j].substr(wCommonPrefix.size(), wComparisonWidth)) {
                        wGroups[wNumOfGroups][wIndex] = iInputData[j];
                        wGroupRegionIDs[wNumOfGroups][wIndex] = iRegionIDs[j];
                        j++;
                        wIndex++;
                    } else {
                        break;
                    }
                }
                wGroupRegionIDs[wNumOfGroups][wIndex] = iRegionIDs[j];
            }
            wNumOfGroups++;
        }
    } else {
        int wIndex = 0;
        for (int i = 0; i < wNumOfGroupBounds; i++) {
            wPositions[i] = iPos + (int)wCommonPrefix.size() + wComparisonWidth;
            wNumOfBoundsPerGroup[i] = wNumOfBoundsPerRegion[i];
            wGroups[i] = new BitString[wNumOfBoundsPerGroup[i]];
            int *wIds = new int[wNumOfBoundsPerGroup[i] + 1];
            for (int k = 0; k < wNumOfBoundsPerGroup[i]; k++) {
                wGroups[i][k] = iInputData[wIndex];
                wIds[k] = iRegionIDs[wIndex];
                wIndex++;
            }
            wIds[wNumOfBoundsPerGroup[i]] = iRegionIDs[wIndex];
            wGroupRegionIDs[i] = wIds;
        }
        wNumOfGroups = wNumOfGroupBounds;
    }

    if (VERBOSE_TRIE) {
        // cout << "\nGROUPS:" << endl;
        for (int i = 0; i < wNumOfGroups; i++) {
            for (j = 0; j < wNumOfBoundsPerGroup[i]; j++) {
                // cout << i << "." << j << " " << wGroups[i][j] << endl;
            }
        }

        // cout << "\nREGION IDS:" << endl;
        for (int i = 0; i < wNumOfGroups; i++) {
            for (j = 0; j < wNumOfBoundsPerGroup[i] + 1; j++) {
                // cout << i << "." << j << " " << wGroupRegionIDs[i][j] << endl;
            }
        }

        // cout << "\nPOSITIONS:" << endl;
        for (int i = 0; i < wNumOfGroups; i++) {
            // cout << i << " - " << wPositions[i] << endl;
        }
    }

    bool wMaxBranchLimitReached = false;

    while (wNumOfGroups > wMostBranches) {
        wMaxBranchLimitReached = true;
        int wNewNumOfGroups = 0;
        BitString **wNewGroups = new BitString*[wNumOfGroups/2 + 1];
        int **wNewGroupRegionIDs = new int*[wNumOfGroups/2 + 1];
        for (int i = 0; i < wNumOfGroups - 1; i++) {
            wNewGroups[wNewNumOfGroups] = new BitString[wNumOfBoundsPerGroup[i] + wNumOfBoundsPerGroup[i + 1]];
            wNewGroupRegionIDs[wNewNumOfGroups] = new int[wNumOfBoundsPerGroup[i] + wNumOfBoundsPerGroup[i + 1] + 1];
            int wIndex = 0;
            for (int j = 0; j < wNumOfBoundsPerGroup[i]; j++) {
                wNewGroups[wNewNumOfGroups][wIndex] = wGroups[i][j];
                wNewGroupRegionIDs[wNewNumOfGroups][wIndex] = wGroupRegionIDs[i][j];
                wIndex++;
            }
            i++;
            for (int j = 0; j < wNumOfBoundsPerGroup[i]; j++) {
                wNewGroups[wNewNumOfGroups][wIndex] = wGroups[i][j];
                wNewGroupRegionIDs[wNewNumOfGroups][wIndex] = wGroupRegionIDs[i][j];
                wIndex++;
            }
            wPositions[wNewNumOfGroups] = iPos + (int)wCommonPrefix.size();
            wNewGroupRegionIDs[wNewNumOfGroups][wIndex] = wGroupRegionIDs[i][wNumOfBoundsPerGroup[i]];
            wNumOfBoundsPerGroup[wNewNumOfGroups] = wNumOfBoundsPerGroup[i - 1] + wNumOfBoundsPerGroup[i];
            wNewNumOfGroups++;
        }
        if (wNumOfGroups%2 > 0) {
            wNewGroups[wNewNumOfGroups] = new BitString[wNumOfBoundsPerGroup[wNumOfGroups - 1]];
            wNewGroupRegionIDs[wNewNumOfGroups] = new int[wNumOfBoundsPerGroup[wNumOfGroups -1 ] + 1];
            for (int i = 0; i < wNumOfBoundsPerGroup[wNumOfGroups - 1]; i++) {
                wNewGroups[wNewNumOfGroups][i] = wGroups[wNumOfGroups - 1][i];
                wNewGroupRegionIDs[wNewNumOfGroups][i] = wGroupRegionIDs[wNumOfGroups - 1][i];
            }
            wPositions[wNewNumOfGroups] = iPos + (int)wCommonPrefix.size();
            wNewGroupRegionIDs[wNewNumOfGroups][wNumOfBoundsPerGroup[wNumOfGroups - 1]] = wGroupRegionIDs[wNumOfGroups - 1][wNumOfBoundsPerGroup[wNumOfGroups - 1]];
            wNumOfBoundsPerGroup[wNewNumOfGroups] = wNumOfBoundsPerGroup[wNumOfGroups - 1];
            wNewNumOfGroups++;
        }

        // Memory leak!!!
        delete [] wGroupRegionIDs;
        delete [] wGroups;
        /*for (int i = 0; i < wNumOfGroups; i++) {
         delete [] wGroupRegionIDs[i];
         delete [] wGroups[i];
         }*/
        wGroups = wNewGroups;
        wGroupRegionIDs = wNewGroupRegionIDs;
        wNumOfGroups = wNewNumOfGroups;
    }

    if (VERBOSE_TRIE) {
        // cout << "\nGROUPS:" << endl;
        for (int i = 0; i < wNumOfGroups; i++) {
            for (j = 0; j < wNumOfBoundsPerGroup[i]; j++) {
                // cout << i << "." << j << " " << wGroups[i][j] << endl;
            }
        }

        // cout << "\nREGION IDS:" << endl;
        for (int i = 0; i < wNumOfGroups; i++) {
            for (j = 0; j < wNumOfBoundsPerGroup[i] + 1; j++) {
                // cout << i << "." << j << " " << wGroupRegionIDs[i][j] << endl;
            }
        }

        // cout << "\nPOSITIONS:" << endl;
        for (int i = 0; i < wNumOfGroups; i++) {
            // cout << i << " - " << wPositions[i] << endl;
        }
    }

    // Create Node
    if (wCommonPrefix != "") {
        iNode->setCommonPrefix(wCommonPrefix.toInt());
    } else {
        iNode->setCommonPrefix(-1);
    }
    iNode->setCommonPrefixLength((unsigned int)wCommonPrefix.size());
    if (wCommonSuffix != "" && mPrefixLength - iPos - wCommonPrefix.size() - wComparisonWidth - wCommonSuffix.size() == 0 && !wMaxBranchLimitReached) {
        iNode->setCommonSuffix(wCommonSuffix.toInt());
        iNode->setCommonSuffixLength((unsigned int)wCommonSuffix.size());
    } else {
        iNode->setCommonSuffix(-1);
        iNode->setCommonSuffixLength(0);
    }

    iNode->setComparePosition((unsigned int)wCommonPrefix.size() + iPos);
    iNode->setCompareLength(wComparisonWidth);

    unsigned int *wComparisons;
    int wNumOfComparisons = 0;
    if (mPrefixLength - iPos - wCommonPrefix.size() - wComparisonWidth - wCommonSuffix.size() == 0 && !wMaxBranchLimitReached) {
        wComparisons = new unsigned int[wNumOfGroups];
        wNumOfComparisons = wNumOfGroups;
        for (int i = 0; i < wNumOfGroups; i++) {
            wComparisons[i] = wGroups[i][0].substr(wCommonPrefix.size(), wComparisonWidth).toInt();
        }
    } else if (wNumOfGroups < wMostBranches) {
        wComparisons = new unsigned int[wNumOfGroups - 1];
        wNumOfComparisons = wNumOfGroups - 1;
        for (int i = 1; i < wNumOfGroups; i++) {
            wComparisons[i - 1] = wGroups[i][0].substr(wCommonPrefix.size(), wComparisonWidth).toInt();
        }
    } else {
        wComparisons = new unsigned int[wNumOfGroups - 1];
        wNumOfComparisons = wNumOfGroups - 1;
        for (int i = 1; i < wNumOfGroups; i++) {
            BitString temp = wGroups[i][0];
            wComparisons[i - 1] = wGroups[i][0].substr(wCommonPrefix.size(), wComparisonWidth).toInt();
        }
    }
    iNode->setComparisons(wComparisons);
    iNode->setNumOfComparisons(wNumOfComparisons);

    //*******************************//
    // No Address Alignment for now!//
    //******************************//
    iNode->setAddressAlignment(false);

    if (VERBOSE_TRIE) {
        // cout << "\nNODE " << NODECOUNT++ << ": " << endl;
        for (int i = 0; i < wNumOfComparisons; i++) {
            // cout << BitString::intToBitString(iNode->getComparisons()[i], wComparisonWidth) << endl;
        }
    }

    if (mPrefixLength - iPos - wCommonPrefix.size() - wComparisonWidth - wCommonSuffix.size() > 0 || wMaxBranchLimitReached) {
        iNode->setNextNodes(new RangeTrieNode[wNumOfGroups + 1]);
        // Recursively call function for every new node
        for (int i = 0; i < wNumOfGroups; i++) {
            buildTopDown(&iNode->getNextNodes()[i], wGroups[i], wNumOfBoundsPerGroup[i], wGroupRegionIDs[i], wPositions[i], iPos, iMaxLength);
        }
        delete [] iRegionIDs;
    } else {
        iNode->setRegionIDs(iRegionIDs);
    }

    //Memory Leak??!!
    /*for (int i = 0; i < wNumOfGroups; i++) {
     if (wGroups[i]) {
     delete [] wGroups[i];
     }
     }*/
}

/// ==================================================
//
//  Bottom-Up Heuristic for Range Trie Construction
//
/// ==================================================

template <class T>
void RangeTrie<T>::buildBottomUp(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, int iPos) {}

/// ============================================
//
//  Update
//
/// ============================================

template <class T>
void RangeTrie<T>::update(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, typename Trie<T>::Action iAction) {
    if (iAction == Trie<T>::Reconstruct) {
        if (mRoot) {
            delete mRoot;
            mRoot = 0;
        }
        if (mActionTable) {
            delete [] mActionTable;
            mActionTable = 0;
        }
        buildTrie(iRoutingTable, iRoutingTableSize);
    } else if (iAction == Trie<T>::Add) {
        for (int i = 0; i < iRoutingTableSize; i++) {
            insert(iRoutingTable[i].getData(), iRoutingTable[i].getAction(), iRoutingTable[i].getActionSize());
        }
    } else if (iAction == Trie<T>::Remove) {
        for (int i = 0; i < iRoutingTableSize; i++) {
            remove(iRoutingTable[i].getData());
        }
    }
}

/// ============================================
//
//  Exact Prefix Match
//
/// ============================================

template <class T>
T RangeTrie<T>::exactPrefixMatch(BitString iPrefix) const {
    return longestPrefixMatch(iPrefix);
}

/// ============================================
//
//  Longest Prefix Match
//
/// ============================================

template <class T>
T RangeTrie<T>::longestPrefixMatch(BitString iPrefix) const {
    if (!mRoot) {
        return mDefaultAction;
    }
    if (iPrefix.size() < mPrefixLength) {
        for (int i = (int)iPrefix.size(); i < mPrefixLength; i++) {
            iPrefix += '0';
        }
    } else if (iPrefix.size() > mPrefixLength) {
        iPrefix = iPrefix.substr(0, mPrefixLength);
    }

    if (iPrefix.toInt() == 0) {
        return mActionTable[0];
    }

    RangeTrieNode *wNode = mRoot;
    RangeTrieNode *wPreviousNode = wNode;
    int wRegionId = -1;
    while (wNode != 0) {
        wPreviousNode = wNode;
        bool wLeaf = false;
        if (wNode->getRegionIDs() != 0) {
            wLeaf = true;
        }
        if (wNode->getCommonPrefixLength() != 0) {
            unsigned int wPrefixInt = iPrefix.substr(wNode->getComparePosition() - wNode->getCommonPrefixLength(), wNode->getCommonPrefixLength()).toInt();
            if (wPrefixInt < wNode->getCommonPrefix()) {
                if (wLeaf) {
                    wRegionId = wNode->getRegionIDs()[0];
                    break;
                }
                wNode = &wNode->getNextNodes()[0];
                continue;
            } else if (wPrefixInt > wNode->getCommonPrefix()) {
                if (wLeaf) {
                    wRegionId = wNode->getRegionIDs()[wNode->getNumOfComparisons()];
                    break;
                }
                wNode = &wNode->getNextNodes()[wNode->getNumOfComparisons()];
                continue;
            }
        }
        unsigned int wComparePrefix = iPrefix.substr(wNode->getComparePosition(), wNode->getCompareLength()).toInt();
        int wNextNodeIndex = 0;
        if (wComparePrefix < wNode->getComparisons()[0]) {
            if (wLeaf) {
                wRegionId = wNode->getRegionIDs()[0];
                break;
            }
            wNode = &wNode->getNextNodes()[0];
            wNextNodeIndex = 0;
        } else if (wComparePrefix >= wNode->getComparisons()[wNode->getNumOfComparisons() - 1]) {
            if (wLeaf) {
                wRegionId = wNode->getRegionIDs()[wNode->getNumOfComparisons()];
                break;
            }
            wNextNodeIndex = wNode->getNumOfComparisons();
            wNode = &wNode->getNextNodes()[wNode->getNumOfComparisons()];
        } else {
            for (int i = 0; i < wNode->getNumOfComparisons() - 1; i++) {
                if (wComparePrefix >= wNode->getComparisons()[i] && wComparePrefix < wNode->getComparisons()[i + 1]) {
                    if (wLeaf) {
                        wRegionId = wNode->getRegionIDs()[i + 1];
                        break;
                    }
                    wNode = &wNode->getNextNodes()[i + 1];
                    wNextNodeIndex = i + 1;
                    break;
                }
            }
        }
        if (wPreviousNode->getCommonSuffixLength() != 0) {
            unsigned int wSuffixInt = iPrefix.substr(mPrefixLength - wPreviousNode->getCommonSuffixLength()).toInt();
            if (wSuffixInt < wPreviousNode->getCommonSuffix()) {
                if (wNextNodeIndex != 0) {
                    if (wLeaf) {
                        wRegionId = wPreviousNode->getRegionIDs()[wNextNodeIndex - 1];
                        break;
                    }
                    wNode = &wPreviousNode->getNextNodes()[wNextNodeIndex - 1];
                }
            }
        }
        if (wLeaf) {
            break;
        }
    }

    if (wRegionId != -1) {
        return mActionTable[wRegionId];
    }
    return mDefaultAction;
}

/// ============================================
//
//  Insert
//
/// ============================================

template <class T>
void RangeTrie<T>::insert(BitString iPrefix, T iAction, int iActionSize) {
    // Check if the prefix already exists
    if (mPrefixTree->exactPrefixMatch(iPrefix) != -1) {
        mActionTable[(mPrefixTree->exactPrefixMatch(iPrefix))] = iAction;
    } else { // Prefix does not exist
        if (!mRoot) {
            if (mActionTable) {
                delete [] mActionTable;
                mActionTable = 0;
            }
            if (mPrefixTree) {
                delete mPrefixTree;
                mPrefixTree = 0;
            }
            RoutingTableEntry<T> *wTempEntry = new RoutingTableEntry<T>(iPrefix, (int)iPrefix.size(), iAction, iActionSize);
            buildTrie(wTempEntry, 1);
            delete wTempEntry;
        }

        T *wNewActionTable = new T[mActionTableSize + 1];
        for (int i = 0; i < mActionTableSize; i++) {
            wNewActionTable[i] = mActionTable[i];
        }
        wNewActionTable[mActionTableSize] = iAction;
        delete [] mActionTable;
        mActionTable = wNewActionTable;
        mActionTableSize++;

        int wOldId;
        if (mPrefixTree->longestPrefixMatch(iPrefix) == -1) {
            wOldId = -1;
        } else {
            wOldId = mPrefixTree->longestPrefixMatch(iPrefix);
        }
        int wNewId = mActionTableSize - 1;

        mPrefixTree->insert(iPrefix, wNewId, (int)mPrefixTree->getNodeForPrefix(iPrefix)->getActionSize());

        BitString wPrefix = iPrefix;

        if (wPrefix.size() < mPrefixLength) {
            for (int i = (int)wPrefix.size(); i < mPrefixLength; i++) {
                wPrefix += '0';
            }
        } else if (wPrefix.size() > mPrefixLength) {
            wPrefix = wPrefix.substr(0, mPrefixLength);
        }

        BitString wUpperLimitPrefix = wPrefix;
        int wIndex = (int) iPrefix.size() - 1;
        for (; wIndex >= 0; wIndex--) {
            if (wUpperLimitPrefix[wIndex] == 0) {
                wUpperLimitPrefix[wIndex] = '1';
                for (int j = wIndex + 1; j < (int)iPrefix.size(); j++) {
                    wUpperLimitPrefix[j] = '0';
                }
                break;
            }
        }
        if (wIndex == -1) {
            wUpperLimitPrefix = "1";
            for (int i = 0; i < iPrefix.size(); i++) {
                wUpperLimitPrefix += '0';
            }
        }

        RangeTrieNode *wLowerLimitNode = mRoot;
        RangeTrieNode *wUpperLimitNode = mRoot;
        RangeTrieNode *wPreviousNode = mRoot;
        stack<RangeTrieNode*> wLowerVisitedNodes;
        stack<RangeTrieNode*> wUpperVisitedNodes;
        stack<int> wLowerIndexes;
        stack<int> wUpperIndexes;
        int wLowerIndex = 0;
        int wUpperIndex = 0;
        // Traverse using lower bound
        while (wLowerLimitNode != 0) {
            wLowerVisitedNodes.push(wLowerLimitNode);
            wPreviousNode = wLowerLimitNode;
            bool wLeaf = false;
            if (wLowerLimitNode->getRegionIDs() != 0) {
                wLeaf = true;
            }
            wLowerIndex = 0;
            if (wLowerLimitNode->getCommonPrefixLength() != 0) {
                unsigned int wPrefixInt = wPrefix.substr(wLowerLimitNode->getComparePosition() - wLowerLimitNode->getCommonPrefixLength(), wLowerLimitNode->getCommonPrefixLength()).toInt();
                if (wPrefixInt < wLowerLimitNode->getCommonPrefix()) {
                    wLowerIndex = 0;
                    if (!wLeaf) {
                        wLowerLimitNode = &wLowerLimitNode->getNextNodes()[0];
                        continue;
                    } else {
                        break;
                    }
                } else if (wPrefixInt > wLowerLimitNode->getCommonPrefix()) {
                    wLowerIndex = wLowerLimitNode->getNumOfComparisons();
                    if (!wLeaf) {
                        wLowerLimitNode = &wLowerLimitNode->getNextNodes()[wLowerLimitNode->getNumOfComparisons()];
                        continue;
                    } else {
                        break;
                    }
                }
            }
            unsigned int wComparePrefix = wPrefix.substr(wLowerLimitNode->getComparePosition(), wLowerLimitNode->getCompareLength()).toInt();
            if (wComparePrefix < wLowerLimitNode->getComparisons()[0]) {
                if (!wLeaf) {
                    wLowerLimitNode = &wLowerLimitNode->getNextNodes()[0];
                }
                wLowerIndex = 0;
            } else if (wComparePrefix >= wLowerLimitNode->getComparisons()[wLowerLimitNode->getNumOfComparisons() - 1]) {
                if (!wLeaf) {
                    wLowerLimitNode = &wLowerLimitNode->getNextNodes()[wLowerLimitNode->getNumOfComparisons()];
                }
                wLowerIndex = wLowerLimitNode->getNumOfComparisons();
            } else {
                for (int i = 0; i < wLowerLimitNode->getNumOfComparisons() - 1; i++) {
                    if (wComparePrefix >= wLowerLimitNode->getComparisons()[i] && wComparePrefix < wLowerLimitNode->getComparisons()[i + 1]) {
                        if (!wLeaf) {
                            wLowerLimitNode = &wLowerLimitNode->getNextNodes()[i + 1];
                        }
                        wLowerIndex = i + 1;
                        break;
                    }
                }
            }
            if (wPreviousNode->getCommonSuffixLength() != 0) {
                unsigned int wSuffixInt = wPrefix.substr(mPrefixLength - wPreviousNode->getCommonSuffixLength()).toInt();
                if (wSuffixInt < wPreviousNode->getCommonSuffix()) {
                    if (wLowerIndex != 0) {
                        if (!wLeaf) {
                            wLowerLimitNode = &wPreviousNode->getNextNodes()[--wLowerIndex];
                        }
                    }
                }
            }
            if (!wLeaf) {
                wLowerIndexes.push(wLowerIndex);
            } else {
                break;
            }
        }

        bool wCommonPrefixAndSuffix = true;

        // Update Lower Limit Node
        if (wLowerLimitNode->getCommonPrefix() != -1 && wPrefix.substr(wLowerLimitNode->getComparePosition() - wLowerLimitNode->getCommonPrefixLength(), wLowerLimitNode->getComparePosition()).toInt() != wLowerLimitNode->getCommonPrefix()) {
            wCommonPrefixAndSuffix = false;
            BitString wCommonPrefix = BitString::intToBitString(wLowerLimitNode->getCommonPrefix(), wLowerLimitNode->getCommonPrefixLength());
            for (int i = 0; i < wLowerLimitNode->getNumOfComparisons(); i++) {
                wLowerLimitNode->getComparisons()[i] = wCommonPrefix.append(BitString::intToBitString(wLowerLimitNode->getComparisons()[i], wLowerLimitNode->getCompareLength())).toInt();
            }
            wLowerLimitNode->setCompareLength(wLowerLimitNode->getCompareLength() + wLowerLimitNode->getCommonPrefixLength());
            wLowerLimitNode->setComparePosition(wLowerLimitNode->getComparePosition() - wLowerLimitNode->getCommonPrefixLength());
            wLowerLimitNode->setCommonPrefix(-1);
            wLowerLimitNode->setCommonPrefixLength(0);
        }
        if (wLowerLimitNode->getCommonSuffix() != -1 && wPrefix.substr(wLowerLimitNode->getComparePosition() + wLowerLimitNode->getCompareLength()).toInt() != wLowerLimitNode->getCommonSuffix()) {
            wCommonPrefixAndSuffix = false;
            BitString wCommonSuffix = BitString::intToBitString(wLowerLimitNode->getCommonSuffix(), wLowerLimitNode->getCommonSuffixLength());
            for (int i = 0; i < wLowerLimitNode->getNumOfComparisons(); i++) {
                wLowerLimitNode->getComparisons()[i] = BitString::intToBitString(wLowerLimitNode->getComparisons()[i], wLowerLimitNode->getCompareLength()).append(wCommonSuffix).toInt();
            }
            wLowerLimitNode->setCompareLength(wLowerLimitNode->getCompareLength() + wLowerLimitNode->getCommonSuffixLength());
            wLowerLimitNode->setCommonSuffix(-1);
            wLowerLimitNode->setCommonSuffixLength(0);
        }

        bool wLowerBoundExists = false;
        if (wCommonPrefixAndSuffix) {
            for (int i = 0; i < wLowerLimitNode->getNumOfComparisons(); i++) {
                if (wPrefix.substr(wLowerLimitNode->getComparePosition(), wLowerLimitNode->getCompareLength()).toInt() == wLowerLimitNode->getComparisons()[i]) {
                    wLowerBoundExists = true;
                    return;
                }
            }
        }

        if (!wLowerBoundExists) {
            int wNewLowerBound = wPrefix.substr(wLowerLimitNode->getComparePosition(), wLowerLimitNode->getCompareLength()).toInt();
            unsigned int *wNewLowerComparisons = new unsigned int[wLowerLimitNode->getNumOfComparisons() + 1];
            int *wNewLowerRegionIds = new int[wLowerLimitNode->getNumOfComparisons() + 2];
            int j = 0;
            if (wNewLowerBound == 0) {
                // Handle this case!
            } else {
                wNewLowerRegionIds[0] = wLowerLimitNode->getRegionIDs()[0];
                for (int i = 0; i < wLowerLimitNode->getNumOfComparisons() + 1; i++) {
                    if (i == wLowerIndex) {
                        wNewLowerComparisons[i] = wNewLowerBound;
                        wNewLowerRegionIds[i + 1] = mPrefixTree->longestPrefixMatch(wPrefix); // ??
                    } else {
                        wNewLowerComparisons[i] = wLowerLimitNode->getComparisons()[j];
                        wNewLowerRegionIds[i + 1] = wLowerLimitNode->getRegionIDs()[j + 1];
                        j++;
                    }
                }
            }

            delete [] wLowerLimitNode->getComparisons();
            wLowerLimitNode->setComparisons(wNewLowerComparisons);
            wLowerLimitNode->setNumOfComparisons(wLowerLimitNode->getNumOfComparisons() + 1);
            delete [] wLowerLimitNode->getRegionIDs();
            wLowerLimitNode->setRegionIDs(wNewLowerRegionIds);
        }

        // Traverse using upper bound
        while (wUpperLimitNode != 0) {
            wUpperVisitedNodes.push(wUpperLimitNode);
            wPreviousNode = wUpperLimitNode;
            bool wLeaf = false;
            if (wUpperLimitNode->getRegionIDs() != 0) {
                wLeaf = true;
            }
            wUpperIndex = 0;
            if (wUpperLimitNode->getCommonPrefixLength() != 0) {
                unsigned int wPrefixInt = wPrefix.substr(wUpperLimitNode->getComparePosition() - wUpperLimitNode->getCommonPrefixLength(), wUpperLimitNode->getCommonPrefixLength()).toInt();
                if (wPrefixInt < wUpperLimitNode->getCommonPrefix()) {
                    wUpperIndex = 0;
                    if (!wLeaf) {
                        wUpperLimitNode = &wUpperLimitNode->getNextNodes()[0];
                        continue;
                    } else {
                        break;
                    }
                } else if (wPrefixInt > wUpperLimitNode->getCommonPrefix()) {
                    wUpperIndex = wUpperLimitNode->getNumOfComparisons();
                    if (!wLeaf) {
                        wUpperLimitNode = &wUpperLimitNode->getNextNodes()[wUpperLimitNode->getNumOfComparisons()];
                        continue;
                    } else {
                        break;
                    }
                }
            }
            unsigned int wComparePrefix = wPrefix.substr(wUpperLimitNode->getComparePosition(), wUpperLimitNode->getCompareLength()).toInt();
            if (wComparePrefix < wUpperLimitNode->getComparisons()[0]) {
                if (!wLeaf) {
                    wUpperLimitNode = &wUpperLimitNode->getNextNodes()[0];
                }
                wUpperIndex = 0;
            } else if (wComparePrefix >= wUpperLimitNode->getComparisons()[wUpperLimitNode->getNumOfComparisons() - 1]) {
                wUpperIndex = wUpperLimitNode->getNumOfComparisons();
                if (!wLeaf) {
                    wUpperLimitNode = &wUpperLimitNode->getNextNodes()[wUpperLimitNode->getNumOfComparisons()];
                }
            } else {
                for (int i = 0; i < wUpperLimitNode->getNumOfComparisons() - 1; i++) {
                    if (wComparePrefix >= wUpperLimitNode->getComparisons()[i] && wComparePrefix < wUpperLimitNode->getComparisons()[i + 1]) {
                        if (!wLeaf) {
                            wUpperLimitNode = &wUpperLimitNode->getNextNodes()[i + 1];
                        }
                        wUpperIndex = i + 1;
                        break;
                    }
                }
            }
            if (wPreviousNode->getCommonSuffixLength() != 0) {
                unsigned int wSuffixInt = wPrefix.substr(mPrefixLength - wPreviousNode->getCommonSuffixLength()).toInt();
                if (wSuffixInt < wPreviousNode->getCommonSuffix()) {
                    if (wUpperIndex != 0) {
                        if (!wLeaf) {
                            wUpperLimitNode = &wPreviousNode->getNextNodes()[--wUpperIndex];
                        }
                    }
                }
            }
            if (!wLeaf) {
                wUpperIndexes.push(wUpperIndex);
            } else {
                break;
            }
        }

        // Update Upper Limit Node
        if (wUpperLimitNode->getCommonPrefix() != -1 && wUpperLimitPrefix.substr(wUpperLimitNode->getComparePosition() - wUpperLimitNode->getCommonPrefixLength(), wUpperLimitNode->getComparePosition()).toInt() != wUpperLimitNode->getCommonPrefix()) {
            wCommonPrefixAndSuffix = false;
            BitString wCommonPrefix = BitString::intToBitString(wUpperLimitNode->getCommonPrefix(), wUpperLimitNode->getCommonPrefixLength());
            for (int i = 0; i < wUpperLimitNode->getNumOfComparisons(); i++) {
                wUpperLimitNode->getComparisons()[i] = wCommonPrefix.append(BitString::intToBitString(wUpperLimitNode->getComparisons()[i], wUpperLimitNode->getCompareLength())).toInt();
            }
            wUpperLimitNode->setCompareLength(wUpperLimitNode->getCompareLength() + wUpperLimitNode->getCommonPrefixLength());
            wUpperLimitNode->setComparePosition(wUpperLimitNode->getComparePosition() - wUpperLimitNode->getCommonPrefixLength());
            wUpperLimitNode->setCommonPrefix(-1);
            wUpperLimitNode->setCommonPrefixLength(0);
        }
        if (wUpperLimitNode->getCommonSuffix() != -1 && wUpperLimitPrefix.substr(wUpperLimitNode->getComparePosition() + wUpperLimitNode->getCompareLength()).toInt() != wUpperLimitNode->getCommonSuffix()) {
            wCommonPrefixAndSuffix = false;
            BitString wCommonSuffix = BitString::intToBitString(wUpperLimitNode->getCommonSuffix(), wUpperLimitNode->getCommonSuffixLength());
            for (int i = 0; i < wUpperLimitNode->getNumOfComparisons(); i++) {
                wUpperLimitNode->getComparisons()[i] = BitString::intToBitString(wUpperLimitNode->getComparisons()[i], wUpperLimitNode->getCompareLength()).append(wCommonSuffix).toInt();
            }
            wUpperLimitNode->setCompareLength(wUpperLimitNode->getCompareLength() + wUpperLimitNode->getCommonSuffixLength());
            wUpperLimitNode->setCommonSuffix(-1);
            wUpperLimitNode->setCommonSuffixLength(0);
        }

        bool wUpperBoundExists = false;
        if (wCommonPrefixAndSuffix) {
            for (int i = 0; i < wUpperLimitNode->getNumOfComparisons(); i++) {
                if (wUpperLimitPrefix.substr(wUpperLimitNode->getComparePosition(), wUpperLimitNode->getCompareLength()).toInt() == wUpperLimitNode->getComparisons()[i]) {
                    wUpperBoundExists = true;
                    return;
                }
            }
        }

        if (!wUpperBoundExists) {
            int wNewUpperBound = wUpperLimitPrefix.substr(wUpperLimitNode->getComparePosition(), wUpperLimitNode->getCompareLength()).toInt();
            unsigned int *wNewUpperComparisons = new unsigned int[wUpperLimitNode->getNumOfComparisons() + 1];
            int *wNewUpperRegionIds = new int[wUpperLimitNode->getNumOfComparisons() + 2];
            int j = 0;
            if (wUpperIndex == 0) {
                // Handle this case!
            } else {
                wNewUpperRegionIds[0] = wUpperLimitNode->getRegionIDs()[0];
                for (int i = 0; i < wUpperLimitNode->getNumOfComparisons() + 1; i++) {
                    if (i == wUpperIndex) {
                        wNewUpperComparisons[i] = wNewUpperBound;
                        wNewUpperRegionIds[i + 1] = mPrefixTree->longestPrefixMatch(wUpperLimitPrefix); // ??
                    } else {
                        wNewUpperComparisons[i] = wUpperLimitNode->getComparisons()[j];
                        wNewUpperRegionIds[i + 1] = wUpperLimitNode->getRegionIDs()[j + 1];
                        j++;
                    }
                }
            }

            delete [] wUpperLimitNode->getComparisons();
            wUpperLimitNode->setComparisons(wNewUpperComparisons);
            wUpperLimitNode->setNumOfComparisons(wUpperLimitNode->getNumOfComparisons() + 1);
            delete [] wUpperLimitNode->getRegionIDs();
            wUpperLimitNode->setRegionIDs(wNewUpperRegionIds);
        }

        if (wLowerLimitNode == wUpperLimitNode) {
            for (int i = wLowerIndex + 1; i < wUpperIndex; i++) {
                if (wLowerLimitNode->getRegionIDs()[i] == wOldId) {
                    wLowerLimitNode->getRegionIDs()[i] = wNewId;
                }
            }
        } else {
            do {
                for (int i = wLowerLimitNode->getNumOfComparisons(); i > wLowerIndex; i--) {
                    if (wLowerLimitNode->getRegionIDs()[i] == wOldId) {
                        wLowerLimitNode->getRegionIDs()[i] = wNewId;
                    }
                }

                for (int i = 0; i < wUpperIndex; i++) {
                    if (wUpperLimitNode->getRegionIDs()[i] == wOldId) {
                        wUpperLimitNode->getRegionIDs()[i] = wNewId;
                    }
                }

                wLowerLimitNode = wLowerVisitedNodes.top();
                wUpperLimitNode = wUpperVisitedNodes.top();
                wLowerIndex = wLowerIndexes.top();
                wUpperIndex = wUpperIndexes.top();
                wLowerVisitedNodes.pop();
                wUpperVisitedNodes.pop();
                wLowerIndexes.pop();
                wUpperIndexes.pop();
            } while (wLowerLimitNode != wUpperLimitNode);

            for (int i = wLowerIndex + 1; i < wUpperIndex; i++) {
                changeRegionId(&wLowerLimitNode->getNextNodes()[i], wOldId, wNewId);
            }
        }
    }
}

/// ============================================
//
//  Remove
//
/// ============================================

template <class T>
void RangeTrie<T>::remove(BitString iPrefix) {
    BitString wPrefix = iPrefix;

    if (iPrefix.size() <= mPrefixLength) {
        for (int i = 0; i < mPrefixLength - iPrefix.size(); i++) {
            wPrefix += '0';
        }
    } else {
        wPrefix = wPrefix.substr(0, mPrefixLength);
    }

    if (mPrefixTree->exactPrefixMatch(iPrefix) == -1) {    // Check if it exists
        return;
    } else if (mPrefixTree->getNodeForPrefix(iPrefix)->getOneNode() == 0 && mPrefixTree->getNodeForPrefix(iPrefix)->getZeroNode() == 0) {   // Check if it is a leaf
        RangeTrieNode *wNode = getNodeForPrefix(iPrefix);
        int wOldId = mPrefixTree->exactPrefixMatch(iPrefix);
        mPrefixTree->remove(iPrefix);
        int wNewId;
        if (mPrefixTree->longestPrefixMatch(iPrefix) != -1) {
            wNewId = mPrefixTree->longestPrefixMatch(iPrefix);
        } else {
            wNewId = -1;
        }
        if (wNode->getCommonPrefixLength() != 0) {
            unsigned int wPrefixInt = wPrefix.substr(wNode->getComparePosition() - wNode->getCommonPrefixLength(), wNode->getCommonPrefixLength()).toInt();
            if (wPrefixInt < wNode->getCommonPrefix()) {
                if (wNode->getRegionIDs()[0] == wOldId) {
                    mActionTable[wOldId] = mDefaultAction;
                    wNode->getRegionIDs()[0] = wNewId;
                }
                return;
            } else if (wPrefixInt > wNode->getCommonPrefix()) {
                if (wNode->getRegionIDs()[wNode->getNumOfComparisons()] == wOldId) {
                    mActionTable[wOldId] = mDefaultAction;
                    wNode->getRegionIDs()[wNode->getNumOfComparisons()] = wNewId;
                }
                return;
            }
        }
        unsigned int wComparePrefix = wPrefix.substr(wNode->getComparePosition(), wNode->getCompareLength()).toInt();
        int wNextNodeIndex = 0;
        int *wCurrentId = 0;
        if (wComparePrefix < wNode->getComparisons()[0]) {
            wCurrentId = &wNode->getRegionIDs()[0];
            wNextNodeIndex = 0;
        } else if (wComparePrefix >= wNode->getComparisons()[wNode->getNumOfComparisons() - 1]) {
            wCurrentId = &wNode->getRegionIDs()[wNode->getNumOfComparisons()];
            wNextNodeIndex = wNode->getNumOfComparisons();
        } else {
            for (int i = 0; i < wNode->getNumOfComparisons() - 1; i++) {
                if (wComparePrefix >= wNode->getComparisons()[i] && wComparePrefix < wNode->getComparisons()[i + 1]) {
                    wCurrentId = &wNode->getRegionIDs()[i + 1];
                    wNextNodeIndex = i + 1;
                    break;
                }
            }
        }
        if (wNode->getCommonSuffixLength() != 0) {
            unsigned int wSuffixInt = wPrefix.substr(mPrefixLength - wNode->getCommonSuffixLength()).toInt();
            if (wSuffixInt < wNode->getCommonSuffix()) {
                if (wNextNodeIndex != 0) {
                    if (wNode->getRegionIDs()[wNextNodeIndex - 1] == wOldId) {
                        mActionTable[wOldId] = mDefaultAction;
                        wNode->getRegionIDs()[wNextNodeIndex - 1] = wNewId;
                    }
                    return;
                }
            }
        }
        if (wCurrentId != 0) {
            if (*wCurrentId == wOldId) {
                mActionTable[wOldId] = mDefaultAction;
                *wCurrentId = wNewId;
            }
        }
        return;
    } else {   // Exists but is not a leaf
        BitString wUpperLimitPrefix = wPrefix;
        int wIndex = (int) iPrefix.size() - 1;
        for (; wIndex >= 0; wIndex--) {
            if (wUpperLimitPrefix[wIndex] == 0) {
                wUpperLimitPrefix[wIndex] = 1;
                for (int j = wIndex + 1; j < (int)iPrefix.size(); j++) {
                    wUpperLimitPrefix[j] = '0';
                }
                break;
            }
        }
        if (wIndex == -1) {
            wUpperLimitPrefix = "1";
            for (int i = 0; i < iPrefix.size(); i++) {
                wUpperLimitPrefix += '0';
            }
        }
        wUpperLimitPrefix = BitString::intToBitString(wUpperLimitPrefix.toInt() - 1, mPrefixLength);
        RangeTrieNode *wLowerLimitNode = getNodeForPrefix(wPrefix);
        RangeTrieNode *wUpperLimitNode = getNodeForPrefix(wUpperLimitPrefix);

        int wId = mPrefixTree->exactPrefixMatch(iPrefix);
        mPrefixTree->remove(iPrefix);
        int wNewId = 0;
        if (mPrefixTree->longestPrefixMatch(iPrefix) != -1) {
            wNewId = mPrefixTree->longestPrefixMatch(iPrefix);
        } else {
            wNewId = -1;
        }

        mActionTable[wId] = mDefaultAction;

        // Check if all changes to be made are in the same node
        if (wLowerLimitNode == wUpperLimitNode) {
            for (int i = 0; i < wLowerLimitNode->getNumOfComparisons(); i++) {
                if (wLowerLimitNode->getRegionIDs()[i] == wId) {
                    wLowerLimitNode->getRegionIDs()[i] = wNewId;
                }
            }
        } else {
            wLowerLimitNode = mRoot;
            wUpperLimitNode = mRoot;
            RangeTrieNode *wPreviousNode = mRoot;
            int wLowerNextNodeIndex = 0;
            int wUpperNextNodeIndex = 0;
            while (wLowerLimitNode == wUpperLimitNode) {
                if (wPreviousNode->getCommonPrefixLength() != 0) {
                    unsigned int wLowerPrefixInt = wPrefix.substr(wPreviousNode->getComparePosition() - wPreviousNode->getCommonPrefixLength(), wPreviousNode->getCommonPrefixLength()).toInt();
                    unsigned int wUpperPrefixInt = wUpperLimitPrefix.substr(wPreviousNode->getComparePosition() - wPreviousNode->getCommonPrefixLength(), wPreviousNode->getCommonPrefixLength()).toInt();

                    // Check common prefix with lower limit prefix
                    if (wLowerPrefixInt < wPreviousNode->getCommonPrefix()) {
                        wLowerLimitNode = &wPreviousNode->getNextNodes()[0];
                        wLowerNextNodeIndex = 0;
                    } else if (wLowerPrefixInt > wPreviousNode->getCommonPrefix()) {
                        wLowerLimitNode = &wPreviousNode->getNextNodes()[wPreviousNode->getNumOfComparisons()];
                        wLowerNextNodeIndex = wPreviousNode->getNumOfComparisons();
                    }

                    // Check common prefix with upper limit prefix
                    if (wUpperPrefixInt < wPreviousNode->getCommonPrefix()) {
                        wUpperLimitNode = &wPreviousNode->getNextNodes()[0];
                        wUpperNextNodeIndex = 0;
                    } else if (wUpperPrefixInt > wPreviousNode->getCommonPrefix()) {
                        wUpperLimitNode = &wPreviousNode->getNextNodes()[wPreviousNode->getNumOfComparisons()];
                        wUpperNextNodeIndex = wPreviousNode->getNumOfComparisons();
                    }
                }
                unsigned int wLowerComparePrefix = wPrefix.substr(wPreviousNode->getComparePosition(), wPreviousNode->getCompareLength()).toInt();
                unsigned int wUpperComparePrefix = wUpperLimitPrefix.substr(wPreviousNode->getComparePosition(), wPreviousNode->getCompareLength()).toInt();

                if (wLowerLimitNode == wPreviousNode) {
                    // Lower Limit Prefix
                    if (wLowerComparePrefix < wPreviousNode->getComparisons()[0]) {
                        wLowerLimitNode = &wPreviousNode->getNextNodes()[0];
                        wLowerNextNodeIndex = 0;
                    } else if (wLowerComparePrefix >= wPreviousNode->getComparisons()[wPreviousNode->getNumOfComparisons() - 1]) {
                        wLowerLimitNode = &wPreviousNode->getNextNodes()[wPreviousNode->getNumOfComparisons()];
                        wLowerNextNodeIndex = wPreviousNode->getNumOfComparisons();
                    } else {
                        for (int i = 0; i < wPreviousNode->getNumOfComparisons() - 1; i++) {
                            if (wLowerComparePrefix >= wPreviousNode->getComparisons()[i] && wLowerComparePrefix < wPreviousNode->getComparisons()[i + 1]) {
                                wLowerLimitNode = &wPreviousNode->getNextNodes()[i + 1];
                                wLowerNextNodeIndex = i + 1;
                                break;
                            }
                        }
                    }
                    if (wPreviousNode->getCommonSuffixLength() != 0) {
                        unsigned int wSuffixInt = wPrefix.substr(mPrefixLength - wPreviousNode->getCommonSuffixLength()).toInt();
                        if (wSuffixInt < wPreviousNode->getCommonSuffix()) {
                            if (wLowerNextNodeIndex != 0) {
                                wLowerLimitNode = &wPreviousNode->getNextNodes()[wLowerNextNodeIndex - 1];
                            }
                        }
                    }
                }

                if (wUpperLimitNode == wPreviousNode) {
                    // Upper Limit Prefix
                    if (wUpperComparePrefix < wPreviousNode->getComparisons()[0]) {
                        wUpperLimitNode = &wPreviousNode->getNextNodes()[0];
                        wUpperNextNodeIndex = 0;
                    } else if (wUpperComparePrefix >= wPreviousNode->getComparisons()[wPreviousNode->getNumOfComparisons() - 1]) {
                        wUpperLimitNode = &wPreviousNode->getNextNodes()[wPreviousNode->getNumOfComparisons()];
                        wUpperNextNodeIndex = wPreviousNode->getNumOfComparisons();
                    } else {
                        for (int i = 0; i < wPreviousNode->getNumOfComparisons() - 1; i++) {
                            if (wUpperComparePrefix >= wPreviousNode->getComparisons()[i] && wUpperComparePrefix < wPreviousNode->getComparisons()[i + 1]) {
                                wUpperLimitNode = &wPreviousNode->getNextNodes()[i + 1];
                                wUpperNextNodeIndex = i + 1;
                                break;
                            }
                        }
                    }
                    if (wPreviousNode->getCommonSuffixLength() != 0) {
                        unsigned int wSuffixInt = wUpperLimitPrefix.substr(mPrefixLength - wPreviousNode->getCommonSuffixLength()).toInt();
                        if (wSuffixInt < wPreviousNode->getCommonSuffix()) {
                            if (wUpperNextNodeIndex != 0) {
                                wUpperLimitNode = &wPreviousNode->getNextNodes()[wUpperNextNodeIndex - 1];
                            }
                        }
                    }
                }

                // Update Previous Nodes
                if (wUpperLimitNode == wLowerLimitNode) {
                    wPreviousNode = wUpperLimitNode;
                    wLowerNextNodeIndex = 0;
                    wUpperNextNodeIndex = 0;
                }
            }

            // Change Ids of all nodes between the lower limit index and upper limit index
            for (int i = wLowerNextNodeIndex; i <= wUpperNextNodeIndex; i++) {
                changeRegionId(&wPreviousNode->getNextNodes()[i], wId, wNewId);
            }


        }

    }

    // Reduce size of next hop table if possible
    int wCount = 0;
    for (int i = mActionTableSize - 1; i >=0; i--) {
        if (mActionTable[i] == mDefaultAction) {
            wCount++;
        } else {
            break;
        }
    }

    if (wCount > 0) {
        T *wNewActionTable = new T[mActionTableSize - wCount];
        for (int i = 0; i < mActionTableSize - wCount; i++) {
            wNewActionTable[i] = mActionTable[i];
        }
        delete [] mActionTable;
        mActionTable = wNewActionTable;
        mActionTableSize -= wCount;
    }

}

/// ===================================================
//
//  Find Max Length of given RoutingTableEntry Array
//
/// ===================================================

template <class T>
int RangeTrie<T>::findMaxLength(RoutingTableEntry<T> *iEntries, int iNum) {
    int wMax = iEntries[0].getLength();
    for (int i = 1; i < iNum; i++) {
        if (iEntries[i].getLength() > wMax) {
            wMax = iEntries[i].getLength();
        }
    }
    return wMax;
}

/// ===========================================================================================
//
//  Extract
//
//  Returns the decimal value of the iBranch next bits from iString starting at iPos
//
/// ===========================================================================================

template <class T>
int RangeTrie<T>::extract(int iPos, int iBranch, BitString iString) const {
    if ((iPos + iBranch) > iString.size()) {
        return 0;
    }
    BitString wString = iString.substr(iPos, iBranch);
    int temp =  wString.toInt();
    return temp;
}

/// ============================================
//
//  Get Node For Prefix
//
/// ============================================

template <class T>
RangeTrieNode* RangeTrie<T>::getNodeForPrefix(BitString iPrefix) const {
    if (!mRoot) {
        return 0;
    }
    if (iPrefix.size() < mPrefixLength) {
        for (int i = (int)iPrefix.size(); i < mPrefixLength; i++) {
            iPrefix += '0';
        }
    } else if (iPrefix.size() > mPrefixLength) {
        iPrefix = iPrefix.substr(0, mPrefixLength);
    }

    RangeTrieNode *wNode = mRoot;
    RangeTrieNode *wPreviousNode = wNode;
    while (wNode != 0) {
        wPreviousNode = wNode;
        if (wNode->getRegionIDs() != 0) {
            return wNode;
        }
        if (wNode->getCommonPrefixLength() != 0) {
            unsigned int wPrefixInt = iPrefix.substr(wNode->getComparePosition() - wNode->getCommonPrefixLength(), wNode->getCommonPrefixLength()).toInt();
            if (wPrefixInt < wNode->getCommonPrefix()) {
                wNode = &wNode->getNextNodes()[0];
                continue;
            } else if (wPrefixInt > wNode->getCommonPrefix()) {
                wNode = &wNode->getNextNodes()[wNode->getNumOfComparisons()];
                continue;
            }
        }
        unsigned int wComparePrefix = iPrefix.substr(wNode->getComparePosition(), wNode->getCompareLength()).toInt();
        int wNextNodeIndex = 0;
        if (wComparePrefix < wNode->getComparisons()[0]) {
            wNode = &wNode->getNextNodes()[0];
            wNextNodeIndex = 0;
        } else if (wComparePrefix >= wNode->getComparisons()[wNode->getNumOfComparisons() - 1]) {
            wNode = &wNode->getNextNodes()[wNode->getNumOfComparisons()];
            wNextNodeIndex = wNode->getNumOfComparisons();
        } else {
            for (int i = 0; i < wNode->getNumOfComparisons() - 1; i++) {
                if (wComparePrefix >= wNode->getComparisons()[i] && wComparePrefix < wNode->getComparisons()[i + 1]) {
                    wNode = &wNode->getNextNodes()[i + 1];
                    wNextNodeIndex = i + 1;
                    break;
                }
            }
        }
        if (wPreviousNode->getCommonSuffixLength() != 0) {
            unsigned int wSuffixInt = iPrefix.substr(mPrefixLength - wPreviousNode->getCommonSuffixLength()).toInt();
            if (wSuffixInt < wPreviousNode->getCommonSuffix()) {
                if (wNextNodeIndex != 0) {
                    wNode = &wPreviousNode->getNextNodes()[wNextNodeIndex - 1];
                }
            }
        }
    }
    return 0;
}

template <class T>
void RangeTrie<T>::changeRegionId(RangeTrieNode *iNode, int iOldId, int iNewId) {
    if (iNode->getRegionIDs() != 0) {
        for (int i = 0; i <= iNode->getNumOfComparisons(); i++) {
            if (iNode->getRegionIDs()[i] == iOldId) {
                iNode->getRegionIDs()[i] = iNewId;
            }
        }
    } else {
        for (int i = 0; i <= iNode->getNumOfComparisons(); i++) {
            changeRegionId(&iNode->getNextNodes()[i], iOldId, iNewId);
        }
    }
}

#endif /* defined(__Trie_Data_Structure__RangeTrie__) */
