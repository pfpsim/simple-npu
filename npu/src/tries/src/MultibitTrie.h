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
//  MultibitTrie.h
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-03-12.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#ifndef __Trie_Data_Structure__MTrie__
#define __Trie_Data_Structure__MTrie__

#include <vector>
#include "MultibitNode.h"
#include "Trie.h"
#include "PrefixTree.h"

using namespace std;

template <class T>
class MultibitTrie : public Trie<T> {
public:

    // Constructor
    MultibitTrie(RoutingTableEntry<T> *iEntryArray, unsigned int iNumOfPrefixes, T iDefaultAction, int iDefaultActionSize, unsigned int iHeight = 4);

    // Destructor
    virtual ~MultibitTrie();

    // Update
    virtual void update(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, typename Trie<T>::Action iAction);

    // Insert
    virtual void insert(BitString iPrefix, T iAction, int iActionSize);

    // Remove
    virtual void remove(BitString iPrefix);

    // Lookup
    virtual T exactPrefixMatch(BitString iPrefix) const;
    virtual T longestPrefixMatch(BitString iPrefix) const;

private:
    MultibitNode<T> *mRoot;
    unsigned int mRootStride;       // Number of bits for root level
    unsigned int mHeight;           // Number of levels
    PrefixTree<T> *mPrefixTree;     // Prefix Tree/ Unibit Trie to keep track of original prefixes
    T mDefaultAction;               // Default Action
    int mDefaultActionSize;         // Default Action Size

    int Opt(UnibitNode<T> *iNode, int iLevels) const;
    int Opt(UnibitNode<T> *iNode, int iStride, int iLevels) const;
    int Height(UnibitNode<T> *iNode) const;
    int HeightHelper(UnibitNode<T> *iNode, int iNum) const;
    int MinIntIndex(int *wArray, int iSize) const;
    int MinInt(int *wArray, int iSize) const;

};

//
//  MultibitTrie.cpp
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-03-12.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

//#include "MultibitTrie.h"
#include <cmath>

/// ============================================
//
//  Constructor
//
/// ============================================

template <class T>
MultibitTrie<T>::MultibitTrie(RoutingTableEntry<T> *iEntryArray, unsigned int iNumOfPrefixes, T iDefaultAction, int iDefaultActionSize, unsigned int iHeight) : mDefaultAction(iDefaultAction), mDefaultActionSize(iDefaultActionSize), mHeight(iHeight) {
    // Build Prefix Tree
    mPrefixTree = new PrefixTree<T>(iDefaultAction, iDefaultActionSize);
    for (int i = 0; i < iNumOfPrefixes; i++) {
        mPrefixTree->insert(iEntryArray[i].getData(), iEntryArray[i].getAction(), iEntryArray[i].getActionSize());
    }

    // Find optimal root stride length
    mRootStride = Opt(mPrefixTree->getRoot(), mHeight);

    // Build root node
    mRoot = new MultibitNode<T>[(int)pow(2, mRootStride)]();

    // Build Multibit Trie
    for (int i = 0; i < iNumOfPrefixes; i++) {
        insert(iEntryArray[i].getData(), iEntryArray[i].getAction(), iEntryArray[i].getActionSize());
    }
}

/// ============================================
//
//  Destructor
//
/// ============================================

template <class T>
MultibitTrie<T>::~MultibitTrie() {
    if (mRoot) {
        delete [] mRoot;
        mRoot = 0;
    }
    if (mPrefixTree) {
        delete mPrefixTree;
        mPrefixTree = 0;
    }
}

/// ============================================
//
//  Update
//
/// ============================================

template <class T>
void MultibitTrie<T>::update(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, typename Trie<T>::Action iAction) {
    if (iAction == Trie<T>::Add) {
        for (int i = 0; i < iRoutingTableSize; i++) {
            mPrefixTree->insert(iRoutingTable[i].getData(), iRoutingTable[i].getAction(), iRoutingTable[i].getActionSize());
        }
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
            mRoot = 0;
        }
        if (mPrefixTree) {
            delete mPrefixTree;
            mPrefixTree = 0;
        }
        // Build Prefix Tree
        mPrefixTree = new PrefixTree<T>(iRoutingTable, iRoutingTableSize, mDefaultAction, mDefaultActionSize);

        // Find optimal root stride length
        mRootStride = Opt(mPrefixTree->getRoot(), mHeight);

        // Build root node
        mRoot = new MultibitNode<T>[(int)pow(2, mRootStride)]();

        // Build Multibit Trie
        for (int i = 0; i < iRoutingTableSize; i++) {
            insert(iRoutingTable[i].getData(), iRoutingTable[i].getAction(), iRoutingTable[i].getActionSize());
        }
    }
}

/// ============================================
//
//  Insert
//
/// ============================================

template <class T>
void MultibitTrie<T>::insert(BitString iPrefix, T iAction, int iActionSize) {
    // Insert into Prefix Tree
    if (!mPrefixTree) {
        mPrefixTree = new PrefixTree<T>(mDefaultAction, mDefaultActionSize);
    }

    mPrefixTree->insert(iPrefix, iAction, iActionSize);

    // Check root
    if (!mRoot) {
        if (mRootStride == 0) {
            mRootStride = Opt(mPrefixTree->getRoot(), mHeight);
        }
        mRoot = new MultibitNode<T>[(int)(pow(2, mRootStride))];
    }

    // Insert into Multibit Tree
    // Start at root
    int wPos = 0;
    MultibitNode<T> *wNode = 0;
    BitString wSubString = iPrefix.substr(wPos, mRootStride);
    // Check if the prefix has already been traversed. Perform prefix expansion if required
    if (mRootStride >= iPrefix.size()) {
        int wNumExpansions = (int)pow(2, (mRootStride - (int)wSubString.size()));;
        for (int i = 0; i < wNumExpansions; i++) {
            BitString wExpandedPrefix = wSubString + BitString::intToBitString(i, mRootStride - (int)wSubString.size());
            int wIndex = wExpandedPrefix.toInt();
            if (mRoot[wIndex].getLength() <= iPrefix.size()) {
                mRoot[wIndex].setAction(iAction, iActionSize);
                mRoot[wIndex].setLength((unsigned int)iPrefix.size());
            }
        }
        return;
    } else {
        int wIndex = wSubString.toInt();
        wNode = &mRoot[wIndex];
        wPos = mRootStride;
    }
    // Traverse the Rest of the Nodes
    int wHeight = mHeight - 1;      // Reduce current height by 1
    while(wPos < iPrefix.size() && wHeight != 0) {
        int wStride = wNode->getStride();
        if (wStride == 0) {     // indicates that the node does not point to another level.
            UnibitNode<T> *wPrefixNode = mPrefixTree->getRoot();
            for (int j = 0; j < wPos; j++) {
                if (iPrefix[j] == 0) {
                    wPrefixNode = wPrefixNode->getZeroNode();
                } else {
                    wPrefixNode = wPrefixNode->getOneNode();
                }
            }
            wStride = Opt(wPrefixNode, wHeight);        // Compute optimal stride length
            wNode->setStride(wStride);                  // Save the stride length in memory
            wNode->setNextNodePtr(new MultibitNode<T>[(int)pow(2, wStride)]());    // Build next level
        }
        wSubString = iPrefix.substr(wPos, wStride);
        wPos += wStride;
        // Check if the prefix has already been traversed. Perform prefix expansion if required.
        if (wPos >= iPrefix.size()) {
            int wNumExpansions = (int)pow(2, (wStride - (int)wSubString.size()));;
            for (int i = 0; i < wNumExpansions; i++) {
                BitString wExpandedPrefix = wSubString + BitString::intToBitString(i, wStride - (int)wSubString.size());
                int wIndex = wExpandedPrefix.toInt();
                if (wNode->getNextNodePtr()[wIndex].getLength() <= iPrefix.size()) {
                    (wNode->getNextNodePtr())[wIndex].setAction(iAction, iActionSize);
                    (wNode->getNextNodePtr())[wIndex].setLength((unsigned int)iPrefix.size());
                }
            }
            break;
        } else {
            // Traverse to next level
            int wIndex = wSubString.toInt();
            wNode = &(wNode->getNextNodePtr())[wIndex];
            wHeight -= 1;
        }
    }
    if (wHeight == 0) {
        // Case where a prefix is added that would exceed the Trie height with the current stride lengths. How to fix this?
    }
    return;
}

/// ============================================
//
//  Remove
//
/// ============================================

template <class T>
void MultibitTrie<T>::remove(BitString iPrefix) {
    if (iPrefix != "") {
        if (mPrefixTree->exactPrefixMatch(iPrefix) == mDefaultAction) {
            return;     // Can only remove prefixes that were added, not expansions
        }
        int wStride = mRootStride;
        BitString wSubString = iPrefix.substr(0, wStride);
        int wIndex = wSubString.toInt();
        MultibitNode<T> *wNodesTraversed[mHeight - 1];
        int wNumNodesTraversed = 0;
        MultibitNode<T> *wNode = mRoot;
        wNode = &mRoot[wIndex];
        if (wStride == iPrefix.size()) {        // Prefix was not expanded
            wNode->setAction(0, 0);
            wNode->setLength(0);
            mPrefixTree->remove(iPrefix);
            T wLongestMatchAction = mPrefixTree->longestPrefixMatch(iPrefix);
            int wActionSize = mPrefixTree->getNodeForLongestMatchingPrefix(iPrefix)->getActionSize();
            if (wLongestMatchAction != mDefaultAction) {
                wNode->setAction(wLongestMatchAction, wActionSize);
                BitString wLongestPrefix = iPrefix;
                for (int i = 0; i < wStride; i++) {
                    wLongestPrefix.pop_back();
                    if (mPrefixTree->exactPrefixMatch(wLongestPrefix) == wLongestMatchAction) {
                        wNode->setLength((int)wLongestPrefix.size());
                    }
                }
            } else {
                bool wFinishFlag = false;
                for (int i = 0; i < pow(2.0, wStride); i++) {
                    if (mRoot[i].getAction() != mDefaultAction || mRoot[i].getNextNodePtr() != 0) {
                        wFinishFlag = true;
                        break;
                    }
                }
                if (!wFinishFlag) {
                    delete  [] mRoot;
                    mRoot = 0;
                    mRootStride = 0;
                }
            }
        } else if (wStride > iPrefix.size()) {          // The prefix was expanded. Delete expansions
            T wAction = mPrefixTree->exactPrefixMatch(iPrefix);
            mPrefixTree->remove(iPrefix);
            T wLongestMatchAction = mPrefixTree->longestPrefixMatch(iPrefix);
            int wActionSize = mPrefixTree->getNodeForLongestMatchingPrefix(iPrefix)->getActionSize();
            int wLongestMatchPrefixLength = 0;
            if (wLongestMatchAction != mDefaultAction) {
                BitString wLongestPrefix = iPrefix;
                for (int i = 0; i < wStride; i++) {
                    wLongestPrefix.pop_back();
                    if (mPrefixTree->exactPrefixMatch(wLongestPrefix) == wLongestMatchAction) {
                        break;
                    }
                }
                wLongestMatchPrefixLength = (int)wLongestPrefix.size();
            }

            int wNumExpansions = (int)pow(2, (wStride - (int)wSubString.size()));
            for (int i = 0; i < wNumExpansions; i++) {
                BitString wExpandedPrefix = wSubString + BitString::intToBitString(i, wStride - (int)wSubString.size());
                int wIndex = wExpandedPrefix.toInt();
                if (mRoot[wIndex].getAction() == wAction) {
                    mRoot[wIndex].setAction(wLongestMatchAction, wActionSize);
                    mRoot[wIndex].setLength(wLongestMatchPrefixLength);
                }
            }

            if (wLongestMatchAction == mDefaultAction) {
                bool wFinishFlag = false;
                for (int i = 0; i < pow(2.0, wStride); i++) {
                    if (mRoot[i].getAction() != mDefaultAction || mRoot[i].getNextNodePtr() != 0) {
                        wFinishFlag = true;
                        break;
                    }
                }
                if (!wFinishFlag) {
                    delete  [] mRoot;
                    mRoot = 0;
                    mRootStride = 0;
                }
            }
        } else {
            int wPos = wStride;
            while (wPos < iPrefix.size() || wNode == 0) {
                wStride = wNode->getStride();
                if (wStride == 0) {
                    return;
                }
                wSubString = iPrefix.substr(wPos, wStride);
                wPos += wStride;
                wIndex = wSubString.toInt();
                wNodesTraversed[wNumNodesTraversed++] = wNode;
                wNode = &(wNode->getNextNodePtr())[wIndex];
                if (wPos >= iPrefix.size()) {
                    bool wActionFlag = false;
                    if (wPos == iPrefix.size()) {       // Prefix was not expanded.
                        wNode->setAction(0, 0);
                        wNode->setLength(0);
                        mPrefixTree->remove(iPrefix);
                        T wLongestMatchAction = mPrefixTree->longestPrefixMatch(iPrefix);
                        int wActionSize = mPrefixTree->getNodeForLongestMatchingPrefix(iPrefix)->getActionSize();
                        // If a prefix must be expanded to the node that was just deleted
                        for (int i = wNumNodesTraversed - 1; i >= 0; i--) {
                            if (wNodesTraversed[i]->getAction() == wLongestMatchAction) {
                                wActionFlag = true;
                                break;
                            }
                        }
                        if (!wActionFlag) {
                            wNode->setAction(wLongestMatchAction, wActionSize);
                            BitString wLongestPrefix = iPrefix;
                            for (int i = 0; i < wStride; i++) {
                                wLongestPrefix.pop_back();
                                if (mPrefixTree->exactPrefixMatch(wLongestPrefix) == wLongestMatchAction) {
                                    wNode->setLength((int)wLongestPrefix.size());
                                }
                            }
                        }
                    } else {            // The prefix was expanded. Delete expansions
                        wNode = wNodesTraversed[wNumNodesTraversed - 1];
                        T wAction = mPrefixTree->exactPrefixMatch(iPrefix);
                        mPrefixTree->remove(iPrefix);
                        T wLongestMatchAction = mPrefixTree->longestPrefixMatch(iPrefix);
                        int wActionSize = mPrefixTree->getNodeForLongestMatchingPrefix(iPrefix)->getActionSize();
                        for (int i = wNumNodesTraversed - 1; i >= 0; i--) {
                            if (wNodesTraversed[i]->getAction() == wLongestMatchAction) {
                                wActionFlag = true;
                                break;
                            }
                        }
                        BitString wLongestPrefix = iPrefix;
                        if (!wActionFlag) {
                            for (int i = 0; i < wStride; i++) {
                                wLongestPrefix.pop_back();
                                if (mPrefixTree->exactPrefixMatch(wLongestPrefix) == wLongestMatchAction) {
                                    break;
                                }
                            }
                        }
                        int wNumExpansions = (int)pow(2, (wStride - (int)wSubString.size()));
                        for (int i = 0; i < wNumExpansions; i++) {
                            BitString wExpandedPrefix = wSubString + BitString::intToBitString(i, wStride - (int)wSubString.size());
                            int wIndex = wExpandedPrefix.toInt();
                            if (wNode->getNextNodePtr()[wIndex].getAction() == wAction) {
                                wNode->getNextNodePtr()[wIndex].setAction(0, 0);
                                wNode->getNextNodePtr()[wIndex].setLength(0);
                                if (!wActionFlag) {
                                    wNode->getNextNodePtr()[wIndex].setAction(wLongestMatchAction, wActionSize);
                                    wNode->getNextNodePtr()[wIndex].setLength((int)wLongestPrefix.size());
                                    break;
                                }
                            }
                        }
                    }
                    if (wActionFlag) {
                        bool wFinishFlag = false;
                        for (int j = wNumNodesTraversed - 1; j >= 0; j--) {
                            for (int i = 0; i < pow(2.0, wNodesTraversed[j]->getStride()); i++) {
                                if ((((wNodesTraversed[j])->getNextNodePtr())[i]).getActionFlag() || (((wNodesTraversed[j])->getNextNodePtr())[i]).getNextNodePtr() != 0) {
                                    wFinishFlag = true;
                                    break;
                                }
                            }
                            if (wFinishFlag) {
                                break;
                            } else {
                                delete [] ((wNodesTraversed[j])->getNextNodePtr());
                                wNodesTraversed[j]->setNextNodePtr(0);
                                wNodesTraversed[j]->setStride(0);
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
}

/// ============================================
//
//  Exact Prefix Match Lookup
//
/// ============================================

template <class T>
T MultibitTrie<T>::exactPrefixMatch(BitString iPrefix) const {
    if (iPrefix != "") {        // Check that the prefix is valid
        int wStride = 0;
        MultibitNode<T> *wNode = 0;
        int wPos = 0;

        // Start at root
        BitString wSubString = iPrefix.substr(0, mRootStride);
        int wIndex = wSubString.toInt();
        wNode = &mRoot[wIndex];
        if (mRootStride == iPrefix.size()) {
            return wNode->getAction();
        } else if (mRootStride > iPrefix.size()) {
            // Check for expanded prefix?
        } else {
            wPos = mRootStride;
            // Traverse the trie
            while (wPos < iPrefix.size() || wNode == 0) {
                wStride = wNode->getStride();
                if (wStride == 0) {
                    break;
                }
                wSubString = iPrefix.substr(wPos, wStride);
                wPos += wStride;
                wIndex = wSubString.toInt();
                wNode = &(wNode->getNextNodePtr())[wIndex];
                if (wPos == iPrefix.size()) {
                    return wNode->getAction();
                } else if (wPos > iPrefix.size()) {
                    // Check for expanded prefix?
                }
            }
        }
    }
    return mDefaultAction;
}

/// ============================================
//
//  Longest Prefix Match Lookup
//
/// ============================================

template <class T>
T MultibitTrie<T>::longestPrefixMatch(BitString iPrefix) const {
    T wBestMatch;
    if (iPrefix != "") {        // Check that the prefix is valid
        int wStride = 0;
        MultibitNode<T> *wNode = 0;
        int wPos = 0;

        // Start at root
        if (mRootStride > iPrefix.size()) {
            // Check for expanded prefix??
            return mDefaultAction;
        }
        BitString wSubString = iPrefix.substr(0, mRootStride);
        int wIndex = wSubString.toInt();
        wNode = &mRoot[wIndex];
        if(wNode->getActionFlag()) {
            wBestMatch = wNode->getAction();
        } else {
            wBestMatch = mDefaultAction;
        }
        wPos = mRootStride;

        // Traverse the trie
        while (wPos < iPrefix.size() || wNode == 0) {
            wStride = wNode->getStride();
            if (wStride == 0) {
                break;
            }
            wSubString = iPrefix.substr(wPos, wStride);
            if (wSubString.size() < wStride) {
                // Check for expanded prefix?
                break;
            }
            wPos += wStride;
            wIndex = wSubString.toInt();
            wNode = &(wNode->getNextNodePtr())[wIndex];
            if (wNode->getActionFlag()) {
                wBestMatch = wNode->getAction();
            }
        }
    }
    return wBestMatch;
}

/// =======================================================================================
//
//  Find Optimal Stride Length
//
//  Returns stride length corresponding the the lowest cost subtree rooted at iNode
//
/// =======================================================================================

template <class T>
int MultibitTrie<T>::Opt(UnibitNode<T> *iNode, int iLevels) const {
    if (iNode == 0 || (iNode->getZeroNode() == 0 && iNode->getOneNode() == 0)) {
        return 0;
    } else if (iLevels == 1) {
        return 1 + Height(iNode);
    } else if (iLevels < 1) {
        return 0;
    } else {
        int wHeight = Height(iNode);
        int wResultArray[wHeight + 1];
        for (int j = 0; j < wHeight + 1; j++) {
            wResultArray[j] = 0;
        }
        for (int i = 1; i <= 1 + wHeight; i++) {
            wResultArray[i - 1] = (pow(2, i) + Opt(iNode, i, iLevels - 1));
        }
        int wMinIndex = MinIntIndex(wResultArray, wHeight + 1);
        if (wMinIndex <= 17) {
            return wMinIndex;
        } else {
            return 17;
        }
    }
}

/// ======================================================
//
//  Find Optimal Stride Length Helper Function
//
//  Returns lowest cost of for subtree rooted at iNode
//
/// ======================================================

template <class T>
int MultibitTrie<T>::Opt(UnibitNode<T> *iNode, int iStride, int iLevels) const {
    if (iNode == 0 || (iNode->getZeroNode() == 0 && iNode->getOneNode() == 0)) {
        return 0;
    } else if (iLevels == 1 && iStride == 0) {
        return pow(2, 1 + Height(iNode));
    } else if (iLevels < 1) {
        return 0;
    } else if (iStride == 0) {
        int wHeight = Height(iNode);
        int wResultArray[wHeight + 1];
        for (int j = 0; j < wHeight + 1; j++) {
            wResultArray[j] = 0;
        }
        for (int i = 1; i <= 1 + wHeight; i++) {
            wResultArray[i - 1] = (pow(2, i) + Opt(iNode, i, iLevels - 1));
        }
        return MinInt(wResultArray, wHeight + 1);
    } else {
        return (Opt(iNode->getOneNode(), iStride - 1, iLevels) + Opt(iNode->getZeroNode(), iStride - 1, iLevels));
    }
}

/// ============================================================
//
//  Find Height
//
//  Computes distance between iNode and furthest leaf node
//
/// ============================================================

template <class T>
int MultibitTrie<T>::Height(UnibitNode<T> *iNode) const {
    return HeightHelper(iNode, 0) - 1;
}

/// ===============================================
//
//  Find Height Helper Function (for recursion)
//
/// ===============================================

template <class T>
int MultibitTrie<T>::HeightHelper(UnibitNode<T> *iNode, int iNum) const {
    int wZeroHeight = 0;
    int wOneHeight = 0;
    if (iNode->getZeroNode()) {
        wZeroHeight = HeightHelper(iNode->getZeroNode(), iNum + 1);
    }
    if (iNode->getOneNode()) {
        wOneHeight = HeightHelper(iNode->getOneNode(), iNum + 1);
    }
    if (!iNode->getOneNode() && !iNode->getZeroNode()) {
        return iNum;
    } else {
        if (wZeroHeight > wOneHeight) {
            return wZeroHeight;
        } else {
            return wOneHeight;
        }
    }
}

/// ===============================================
//
//  Return index of smallest value in the array
//
/// ===============================================

template <class T>
int MultibitTrie<T>::MinIntIndex(int *wArray, int iSize) const {
    if (iSize > 0) {
        int wMinIndex = 0;
        for (int i = 1; i < iSize; i++) {
            if (wArray[i] < wArray[wMinIndex]) {
                wMinIndex = i;
            }
        }
        return wMinIndex + 1;
    }
    return 0;
}

/// ============================================
//
//  Return smallest value in the array
//
/// ============================================

template <class T>
int MultibitTrie<T>::MinInt(int *wArray, int iSize) const {
    if (iSize > 0) {
        int wMin = wArray[0];
        for (int i = 1; i < iSize; i++) {
            if (wArray[i] < wMin) {
                wMin = wArray[i];
            }
        }
        return wMin;
    }
    return 0;
}

#endif /* defined(__Trie_Data_Structure__MTrie__) */