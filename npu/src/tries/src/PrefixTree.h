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
//  PrefixTree.h
//

#ifndef __PrefixTree__
#define __PrefixTree__

#include "Trie.h"
#include "UnibitNode.h"

template <class T>
class PrefixTree : public Trie<T> {
public:

    // Constructors
    PrefixTree(T iDefaultAction, int iDefaultActionSize);
    PrefixTree(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, T iDefaultAction, int iDefaultActionSize);
    PrefixTree(const PrefixTree &iTree);

    // Destructor
    virtual ~PrefixTree();

    // Update
    virtual void update(RoutingTableEntry<T> *iRoutingTableEntry, int iRoutingTableSize, typename Trie<T>::Action iAction);

    // Insert
    virtual void insert(BitString iPrefix, T iAction, int iActionSize);

    // Remove
    virtual void remove(BitString iPrefix);
    virtual void remove2(BitString iPrefix);

    // Lookup
    virtual T exactPrefixMatch(BitString iPrefix) const;
    virtual T longestPrefixMatch(BitString iPrefix) const;

    // Getter
    UnibitNode<T>* getRoot() const;

    static void countNumberOfNodes(UnibitNode<T> *iNode, int &oNum);

    UnibitNode<T>* getNodeForPrefix(BitString iPrefix) const;
    UnibitNode<T>* getNodeForLongestMatchingPrefix(BitString iPrefix) const;

private:
    UnibitNode<T> *mRoot;
    T mDefaultAction;
    int mDefaultActionSize;
    virtual bool remove2(UnibitNode<T>* iNode, BitString iPrefix, int iIndex);
};

//
//  PrefixTree.cpp
//

//#include "PrefixTree.h"

/// ==========================
//
//  Default Constructors
//
/// ==========================

template <class T>
PrefixTree<T>::PrefixTree(T iDefaultAction, int iDefaultActionSize) : mDefaultAction(iDefaultAction), mDefaultActionSize(iDefaultActionSize) {
  mRoot=new UnibitNode<T>(iDefaultAction, iDefaultActionSize);
}

template <class T>
PrefixTree<T>::PrefixTree(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, T iDefaultAction, int iDefaultActionSize) : PrefixTree(iDefaultAction, iDefaultActionSize) {
    mRoot = new UnibitNode<T>(iDefaultAction, iDefaultActionSize);
    for (int i = 0; i < iRoutingTableSize; i++) {
        insert(iRoutingTable[i].getData(), iRoutingTable[i].getAction(), iRoutingTable[i].getActionSize());
    }

}

/// ==========================
//
//  Copy Constructor
//
/// ==========================

template <class T>
PrefixTree<T>::PrefixTree(const PrefixTree<T> &iTree) = default;

/// ==========================
//
//  Destructor
//
/// ==========================

template <class T>
PrefixTree<T>::~PrefixTree() {
    if (mRoot) {
        delete mRoot;
        mRoot = 0;
    }
}

/// ==========================
//
//  Update
//
/// ==========================

template <class T>
void PrefixTree<T>::update(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, typename Trie<T>::Action iAction) {
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
            delete mRoot;
            mRoot = 0;
        }
        for (int i = 0; i < iRoutingTableSize; i++) {
            insert(iRoutingTable[i].getData(), iRoutingTable[i].getAction(), iRoutingTable[i].getActionSize());
        }
    }
}

/// ==========================
//
//  Insert
//
/// ==========================

template <class T>
void PrefixTree<T>::insert(BitString iPrefix, T iAction, int iActionSize) {
    UnibitNode<T> *wNodePtr = 0;
    if (mRoot) {
        wNodePtr = mRoot;
    } else {
        mRoot = new UnibitNode<T>(mDefaultAction, mDefaultActionSize);
        wNodePtr = mRoot;
    }
    unsigned long wSize = iPrefix.size();
    int i = 0;
    for (; i < wSize; i++) {
        if (iPrefix[i] == 0) {
            if (wNodePtr->getZeroNode()) {
                wNodePtr = wNodePtr->getZeroNode();
            } else {
                if (i == wSize - 1) {
                    wNodePtr->setZeroNode(new UnibitNode<T>(iAction, iActionSize));
                    return;
                } else {
                    wNodePtr->setZeroNode(new UnibitNode<T>());
                    wNodePtr = wNodePtr->getZeroNode();
                }
            }
        } else {
            if (wNodePtr->getOneNode()) {
                wNodePtr = wNodePtr->getOneNode();
            } else {
                if (i == wSize - 1) {
                    wNodePtr->setOneNode(new UnibitNode<T>(iAction, iActionSize));
                    return;
                } else {
                    wNodePtr->setOneNode(new UnibitNode<T>());
                    wNodePtr = wNodePtr->getOneNode();
                }
            }
        }
    }

    if (wSize == 0) {
        mDefaultAction = iAction;
        mDefaultActionSize = iActionSize;
    }
    wNodePtr->setAction(iAction, iActionSize);
}

/// ==============================
//
//  Exact Prefix Match Lookup
//
/// ==============================

template <class T>
T PrefixTree<T>::exactPrefixMatch(BitString iPrefix) const {
    UnibitNode<T> *wNodePtr = 0;
    if (mRoot) {
        wNodePtr = mRoot;
        for (int i = 0; i < iPrefix.size(); i++) {
            if (iPrefix[i] == 0) {
                if (wNodePtr->getZeroNode()) {
                    wNodePtr = wNodePtr->getZeroNode();
                } else {
                    return mDefaultAction;
                }
            } else {
                if (wNodePtr->getOneNode()) {
                    wNodePtr = wNodePtr->getOneNode();
                } else {
                    return mDefaultAction;
                }
            }
        }
        return wNodePtr->getAction();
    }
    return mDefaultAction;
}

/// ===============================
//
//  Longest Prefix Match Lookup
//
/// ===============================

template <class T>
T PrefixTree<T>::longestPrefixMatch(BitString iPrefix) const {
    if (mRoot) {
        T wBestMatchingPrefix = mRoot->getAction();
        UnibitNode<T> *wNodePtr = mRoot;
        for (int i = 0; i < iPrefix.size(); i++) {
            if (iPrefix[i] == 0) {
                if (wNodePtr->getZeroNode()) {
                    wNodePtr = wNodePtr->getZeroNode();
                } else {
                    break;
                }
            } else {
                if (wNodePtr->getOneNode()) {
                    wNodePtr = wNodePtr->getOneNode();
                } else {
                    break;
                }
            }
            if (wNodePtr->getActionFlag()) {
                wBestMatchingPrefix = wNodePtr->getAction();
            }
        }
        return wBestMatchingPrefix;
    } else {
        return mDefaultAction;
    }
}

/// ==========================
//
//  Remove
//
/// ==========================

template <class T>
void PrefixTree<T>::remove(BitString iPrefix) {
    if (0 != iPrefix.size()) {
        UnibitNode<T> *wNodePtrArray[iPrefix.size()];
        if (mRoot) {
            wNodePtrArray[0] = mRoot;
            for (int i = 1; i <= iPrefix.size(); i++) {
                if (iPrefix[i - 1] == 0) {
                    if (0 != wNodePtrArray[i - 1]->getZeroNode()) {
                        wNodePtrArray[i] = wNodePtrArray[i - 1]->getZeroNode();
                    } else {
                        return;
                    }
                } else {
                    if (0 != wNodePtrArray[i - 1]->getOneNode()) {
                        wNodePtrArray[i] = wNodePtrArray[i - 1]->getOneNode();
                    } else {
                        return;
                    }
                }
            }
            int j = (int)iPrefix.size();
            if (wNodePtrArray[j]->getOneNode() == 0 && wNodePtrArray[j]->getZeroNode() == 0) {
                if (wNodePtrArray[j - 1]->getOneNode() == wNodePtrArray[j]) {
                    wNodePtrArray[j - 1]->setOneNode(0);
                } else {
                    wNodePtrArray[j - 1]->setZeroNode(0);
                }
                delete wNodePtrArray[j];
                wNodePtrArray[j] = 0;
            } else {
                wNodePtrArray[j]->setAction(mDefaultAction, mDefaultActionSize, false);
            }
            j--;
            for (; j >= 0; j--) {
                if (wNodePtrArray[j]->getOneNode() == 0 && wNodePtrArray[j]->getZeroNode() == 0 && wNodePtrArray[j]->getActionFlag() == false) {
                    if (j != 0) {
                        if (wNodePtrArray[j - 1]->getOneNode() == wNodePtrArray[j]) {
                            wNodePtrArray[j - 1]->setOneNode(0);
                        } else {
                            wNodePtrArray[j - 1]->setZeroNode(0);
                        }
                        delete wNodePtrArray[j];
                        wNodePtrArray[j] = 0;
                    } else {
                        delete mRoot;
                        mRoot = 0;
                    }
                } else {
                    break;
                }
            }
        }
    }
}

/// ===============================
//
//  Recursive Remove Function
//
/// ===============================

template <class T>
void PrefixTree<T>::remove2(BitString iPrefix) {
    if (0 != iPrefix.size()) {
        remove2(mRoot, iPrefix, 0);
    }
}

template <class T>
bool PrefixTree<T>::remove2(UnibitNode<T> *iNode, BitString iPrefix, int iIndex) {
    if (iNode) {
        if (iIndex == iPrefix.size()) {
            if (!iNode->getOneNode() && !iNode->getZeroNode()) {
                return true;
            } else {
                iNode->setAction(0, 0);
                return false;
            }
        } else {
            if (iPrefix[iIndex] == 0) {
                if (remove2(iNode->getZeroNode(), iPrefix, ++iIndex)) {
                    delete iNode->getZeroNode();
                    iNode->setZeroNode(0);
                }
            } else {
                if (remove2(iNode->getOneNode(), iPrefix, ++iIndex)) {
                    delete iNode->getOneNode();
                    iNode->setOneNode(0);
                }
            }
            return (!iNode->getOneNode() && !iNode->getZeroNode() && !iNode->getActionFlag());
        }
    }
    return false;
}

/// ============================
//
//  Getter
//
/// ============================

template <class T>
UnibitNode<T>* PrefixTree<T>::getRoot() const {
    return mRoot;
}

/// ==========================
//
//  Count Number of Nodes
//
/// ==========================

template <class T>
void PrefixTree<T>::countNumberOfNodes(UnibitNode<T> *iNode, int &oNum) {
    if (iNode) {
        if (iNode->getZeroNode() != 0) {
            countNumberOfNodes(iNode->getZeroNode(), oNum);
        }
        if (iNode->getOneNode() != 0) {
            countNumberOfNodes(iNode->getOneNode(), oNum);
        }
        oNum++;
    }
}

/// =============================================================
//
//  Get Node for Prefix
//
//  Returns a pointer to the node at which this prefix ends
//
/// =============================================================

template <class T>
UnibitNode<T>* PrefixTree<T>::getNodeForPrefix(BitString iPrefix) const {
    UnibitNode<T> *wNodePtr = 0;
    if (mRoot) {
        wNodePtr = mRoot;
        for (int i = 0; i < iPrefix.size(); i++) {
            if (iPrefix[i] == 0) {
                if (wNodePtr->getZeroNode()) {
                    wNodePtr = wNodePtr->getZeroNode();
                } else {
                    return 0;
                }
            } else {
                if (wNodePtr->getOneNode()) {
                    wNodePtr = wNodePtr->getOneNode();
                } else {
                    return 0;
                }
            }
        }
        return wNodePtr;
    }
    return 0;
}

template <class T>
UnibitNode<T>* PrefixTree<T>::getNodeForLongestMatchingPrefix(BitString iPrefix) const {
    if (mRoot) {
        T wBestMatchingPrefix = mRoot->getAction();
        UnibitNode<T> *wNodePtr = 0;
        if (mRoot) {
            wNodePtr = mRoot;
            for (int i = 0; i < iPrefix.size(); i++) {
                if (iPrefix[i] == 0) {
                    if (wNodePtr->getZeroNode()) {
                        wNodePtr = wNodePtr->getZeroNode();
                    } else {
                        break;
                    }
                } else {
                    if (wNodePtr->getOneNode()) {
                        wNodePtr = wNodePtr->getOneNode();
                    } else {
                        break;
                    }
                }
                if (wNodePtr->getActionFlag()) {
                    wBestMatchingPrefix = wNodePtr->getAction();
                }
            }
        }
        return wNodePtr;
    } else {
        return 0;
    }
}


#endif /* defined(__PrefixTree__) */
