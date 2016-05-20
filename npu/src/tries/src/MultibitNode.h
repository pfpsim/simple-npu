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
//  MultibitNode.h
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-03-12.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#ifndef __Trie_Data_Structure__MultibitNode__
#define __Trie_Data_Structure__MultibitNode__

#include "BitString.h"
#include "systemc.h"
template <class T>
class MultibitNode {
public:
    // Constructor
    MultibitNode();

    // Destructor
    ~MultibitNode();

    // Setters
    void setStride(unsigned int iStride);
    void setNextNodePtr(MultibitNode<T> *iNextNode);
    void setAction(T iAction, int iActionSize, bool iActionFlag = true);
    void setActionFlag(bool iFlag);
    void setLength(unsigned int iLength);

    // Getters
    unsigned int getStride() const;
    MultibitNode<T>* getNextNodePtr() const;
    T getAction() const;
    int getActionSize() const;
    bool getActionFlag() const;
    unsigned int getLength() const;

    void* operator new(long unsigned) throw(const char*);
    void* operator new[](long unsigned) throw(const char*);
    void operator delete(void*);
    void operator delete[](void*);

private:
    unsigned int mStride;           // Length of prefixes in its next level
    MultibitNode *mNextNodePtr;     // Pointer to the next level
    T mAction;                      // Action information
    int mActionSize;                // Action Size for TLM memory
    unsigned int mLength;           // Length of the prefix whose next hop was assigned to this node
    bool mActionFlag;               // Boolean indicating if the Action is valid
    int tlm_addr ;

};

//
//  MultibitNode.cpp
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-03-12.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

//#include "MultibitNode.h"

/// ============================
//
//  Constructor
//
/// ============================

template <class T>
MultibitNode<T>::MultibitNode() : mStride(0), mNextNodePtr(0), mLength(0), mActionFlag(false) {
  int addr = tlmsingelton::getInstance().tlmvarptr->allocate_mem(32*5); //write_mem and Construct in tlm memory
  this->tlm_addr = addr;
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(mStride),this->tlm_addr);
  tlmsingelton::getInstance().tlmvarptr->write_mem(reinterpret_cast<std::size_t>(mNextNodePtr),this->tlm_addr+1);
  tlmsingelton::getInstance().tlmvarptr->write_mem(0,this->tlm_addr+2);//mAction
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(mLength),this->tlm_addr+3);
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(mActionFlag),this->tlm_addr+4);
}

/// ============================
//
//  Destructor
//
/// ============================

template <class T>
MultibitNode<T>::~MultibitNode() {}

/// ============================
//
//  Setters
//
/// ============================

template <class T>
void MultibitNode<T>::setStride(unsigned int iStride) {
  mStride = iStride;
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(iStride),this->tlm_addr);
}

template <class T>
void MultibitNode<T>::setNextNodePtr(MultibitNode *iNextNode) {
    mNextNodePtr = iNextNode;
    tlmsingelton::getInstance().tlmvarptr->write_mem(reinterpret_cast<std::size_t>(iNextNode),this->tlm_addr+1);
}

template <class T>
void MultibitNode<T>::setAction(T iAction, int iActionSize, bool iActionFlag) {
    mAction = iAction;
    mActionSize = iActionSize;
    mActionFlag = iActionFlag;
    tlmsingelton::getInstance().tlmvarptr->write_mem(0,this->tlm_addr+2);//mAction
    tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(iActionFlag),this->tlm_addr+4);
}

template <class T>
void MultibitNode<T>::setActionFlag(bool iFlag) {
    mActionFlag = iFlag;
    tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(iFlag),this->tlm_addr+4);
}

template <class T>
void MultibitNode<T>::setLength(unsigned int iLength) {
    mLength = iLength;
    tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(iLength),this->tlm_addr+3);
}

/// ============================
//
//  Getters
//
/// ============================

template <class T>
unsigned int MultibitNode<T>::getStride() const {
  std::size_t val = tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr);
  return mStride;
  // return static_cast<unsigned int>(val);
}

template <class T>
MultibitNode<T>* MultibitNode<T>::getNextNodePtr() const {
  std::size_t val = tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+1);
  // return reinterpret_cast<MultibitNode*>(val);
  return mNextNodePtr;
}

template <class T>
T MultibitNode<T>::getAction() const {
  std::size_t val = tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+2);
  return mAction;
}

template <class T>
int MultibitNode<T>::getActionSize() const {
    return mActionSize;
}
template <class T>
bool MultibitNode<T>::getActionFlag() const {
  std::size_t val = tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+4);
  return mActionFlag;
}

template <class T>
unsigned int MultibitNode<T>::getLength() const {
  std::size_t val = tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+3);
  return mLength;
}

/*
 *------------- Allocation Operators ----------------------------------------------
 */
template <class T>
void* MultibitNode<T>::operator new(long unsigned size) throw(const char*) {

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
void* MultibitNode<T>::operator new[](long unsigned size) throw(const char*) {
	#if debug_node_new
	cout << "   Node::operator new[](" << size << ")\t";
	#endif
	void *p = malloc(size); if (!p) throw "Node::operator new[]() error";
	#if debug_node_new
	cout << static_cast<void*>(p) << endl;
	#endif
	return p;
}
template <class T>
void MultibitNode<T>::operator delete(void *p) {
	MultibitNode *fp = static_cast<MultibitNode*>(p);

	#if debug_node_delete
    cout << "   Node::operator delete(" << p << "):\t" << endl;
	#endif

    free(p);
}
template <class T>
void MultibitNode<T>::operator delete[](void *p) {
	#if debug_node_delete
    cout << "   Node::operator delete[]()" << endl;
    cout << "   Node::operator delete[](" << p << ")" << endl;
	#endif
    free(p);
}


#endif /* defined(__Trie_Data_Structure__MNode__) */
