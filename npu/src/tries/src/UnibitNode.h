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
//  UnibitNode.h
//

#ifndef __UnibitNode__
#define __UnibitNode__

#include "BitString.h"
#include "Trie.h"
#include "systemc.h"
#include "tlmsingletoninclude.h"
using namespace std;

template <class T>
class UnibitNode {
public:

    // Constructor
    UnibitNode(T iAction, int iActionSize, bool iActionFlag = true, UnibitNode *iOneNodePtr = 0, UnibitNode *iZeroNodePtr = 0);
    UnibitNode();
    UnibitNode(const UnibitNode &iCopyNode);

    // Destructor
    virtual ~UnibitNode();

    // Getters
    UnibitNode* getOneNode() const;
    UnibitNode* getZeroNode() const;
    T getAction() const;
    int getActionSize() const;
    bool getActionFlag() const;

    // Setters
    void setOneNode(UnibitNode* iNewNode);
    void setZeroNode(UnibitNode* iNewNode);
    void setAction(T iNewAction, int iActionSize, bool iActionFlag = true);
    void setActionFlag(bool iFlag);

    void* operator new(long unsigned) throw(const char*);
    void* operator new[](long unsigned) throw(const char*);
    void operator delete(void*);
    void operator delete[](void*);

private:
    int tlm_addr;
    T mAction;
    bool mActionFlag;
    int mActionSize;
    UnibitNode *mOneNodePtr;
    UnibitNode *mZeroNodePtr;
};

//
//  UnibitNode.cpp
//

//#include "UnibitNode.h"

/// ==========================
//
//  Default Constructors
//
/// ==========================

template <class T>
UnibitNode<T>::UnibitNode() : mActionFlag(false), mOneNodePtr(0), mZeroNodePtr(0) {
  // cout<<"Default Construction Node1"<<endl;
  int addr = tlmsingelton::getInstance().tlmvarptr->allocate_mem(32*4); //Allocate and Construct in tlm memory
  // cout<<"Default Construction New Node1.0@"<<addr<<endl;
#if debug_whatis_node_struct_base_tlmaddr
  cout<<"      Addr in tlm mem "<<hex<<addr<<endl;
#endif
  this->tlm_addr = addr;
  std::size_t val = 0;
  // cout<<"Default Construction New Node1.1@"<<this->tlm_addr<<endl;
  tlmsingelton::getInstance().tlmvarptr->write_mem(val,this->tlm_addr);
  // cout<<"Default Construction New Node1.2@"<<this->tlm_addr+1<<" val:"<<reinterpret_cast<std::size_t>(mOneNodePtr)<<endl;
  tlmsingelton::getInstance().tlmvarptr->write_mem(reinterpret_cast<std::size_t>(mOneNodePtr),this->tlm_addr+1);
  // cout<<"Default Construction New Node1.3@"<<this->tlm_addr+2<<" val:"<<reinterpret_cast<std::size_t>(mZeroNodePtr)<<endl;
  tlmsingelton::getInstance().tlmvarptr->write_mem(reinterpret_cast<std::size_t>(mZeroNodePtr),this->tlm_addr+2);
  // cout<<"Default Construction New Node1.4@"<<this->tlm_addr+3<<" val:"<<static_cast<std::size_t>(mActionFlag)<<endl;
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(mActionFlag),this->tlm_addr+3);
  // cout<<"Default Construction New Node1.5 Constructed"<<addr<<endl;

}

template <class T>
UnibitNode<T>::UnibitNode(T iAction, int iActionSize, bool iActionFlag, UnibitNode *iOneNodePtr, UnibitNode *iZeroNodePtr) :
mAction(iAction),
mActionSize(iActionSize),
mActionFlag(iActionFlag),
mOneNodePtr(iOneNodePtr),
mZeroNodePtr(iZeroNodePtr) {
  // cout<<"New Node1.0"<<endl;
  int addr = tlmsingelton::getInstance().tlmvarptr->allocate_mem(32*4); //Allocate and Construct in tlm memory
  // cout<<"New Node1.0.1@"<<addr<<endl;
#if debug_whatis_node_struct_base_tlmaddr
  cout<<"      Addr in tlm mem "<<hex<<addr<<endl;
#endif
  this->tlm_addr = addr;
  // cout<<"New Node1.1@"<<this->tlm_addr<<endl;
  tlmsingelton::getInstance().tlmvarptr->write_mem(0,this->tlm_addr);
  // cout<<"New Node1.2@"<<this->tlm_addr+1<<" val:"<<reinterpret_cast<std::size_t>(iOneNodePtr)<<endl;
  tlmsingelton::getInstance().tlmvarptr->write_mem(reinterpret_cast<std::size_t>(iOneNodePtr),this->tlm_addr+1);
  // cout<<"New Node1.3@"<<this->tlm_addr+2<<" val:"<<reinterpret_cast<std::size_t>(iZeroNodePtr)<<endl;
  tlmsingelton::getInstance().tlmvarptr->write_mem(reinterpret_cast<std::size_t>(iZeroNodePtr),this->tlm_addr+2);
  // cout<<"New Node1.4@"<<this->tlm_addr+3<<" val:"<<static_cast<std::size_t>(iActionFlag)<<endl;
  tlmsingelton::getInstance().tlmvarptr->write_mem(static_cast<std::size_t>(iActionFlag),this->tlm_addr+3);
  // cout<<"New Node1.5 Constructed"<<addr<<endl;

}

/// ==========================
//
//  Copy Constructor
//
/// ==========================

template <class T>
UnibitNode<T>::UnibitNode(const UnibitNode &iCopyNode) {
  // cout<<"Node is copyconstructed"<<endl;
    mAction = iCopyNode.mAction;
    mActionSize = iCopyNode.mActionSize;
    mActionFlag = iCopyNode.mActionFlag;
    if (0 != iCopyNode.mOneNodePtr) {
        mOneNodePtr = new UnibitNode(*iCopyNode.mOneNodePtr);
    } else {
        mOneNodePtr = 0;
    }
    if (0 != iCopyNode.mZeroNodePtr) {
        mZeroNodePtr = new UnibitNode(*iCopyNode.mZeroNodePtr);
    } else {
        mZeroNodePtr = 0;
    }
}

/// ==========================
//
//  Destructor
//
/// ==========================

template <class T>
UnibitNode<T>::~UnibitNode() {
    if (0 != mOneNodePtr) {
        delete mOneNodePtr;
        mOneNodePtr = 0;
    }

    if (0 != mZeroNodePtr) {
        delete mZeroNodePtr;
        mZeroNodePtr = 0;
    }
}

/// =========================
//
//  Getters
//
/// =========================

template <class T>
UnibitNode<T>* UnibitNode<T>::getOneNode() const {
  // cout<<"getOneNode"<<this->tlm_addr+1<<endl;
  std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+1,reinterpret_cast<std::size_t>(mOneNodePtr));
  // cout<<"GetOneNodeDiff: "<<reinterpret_cast<UnibitNode<T>*>(val)<<" -- "<<mOneNodePtr<<" @tlmaddr:"<<this->tlm_addr+1<<endl;
  //assert (val==reinterpret_cast<std::size_t>(mOneNodePtr));
  return mOneNodePtr;
  // return reinterpret_cast<UnibitNode<T>*>(val);
  // if (reinterpret_cast<UnibitNode<T>*>(val)!=mOneNodePtr){
  //   cout<<"TLM/Host Conflict - GetOneNode: "<<sc_get_current_process_handle().get_parent_object()->basename()<<endl;
  //   // std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+1);
  // }
  // return temp;

}

template <class T>
UnibitNode<T>* UnibitNode<T>::getZeroNode() const {
  // cout<<"getZeroNode"<<this->tlm_addr+2<<endl;
  std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+2,reinterpret_cast<std::size_t>(mZeroNodePtr));
  // assert (val==reinterpret_cast<std::size_t>(mZeroNodePtr));

  // cout<<"GetZeroNodeDiff: "<<reinterpret_cast<UnibitNode<T>*>(val)<<" -- "<<mZeroNodePtr<<" @tlmaddr:"<<this->tlm_addr+2<<endl;
  return mZeroNodePtr;
  // return reinterpret_cast<UnibitNode*>(val);
}

template <class T>
T UnibitNode<T>::getAction() const {
  // cout<<"getAction"<<this->tlm_addr<<endl;
  std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr);
  return mAction;
}

template <class T>
int UnibitNode<T>::getActionSize() const {
  // cout<<"getActionSize"<<this->tlm_addr<<endl;
  return mActionSize;
}

template<class T>
bool UnibitNode<T>::getActionFlag() const {
    // cout<<"getActionFlag"<<this->tlm_addr+3<<endl;
    std::size_t val =   tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+3,static_cast<std::size_t>(mActionFlag));
    // cout<<"getActionFlagDiff: "<<reinterpret_cast<UnibitNode<T>*>(val)<<" -- "<<mActionFlag<<" @tlmaddr:"<<this->tlm_addr+3<<endl;
    return mActionFlag;
    // return static_cast<bool>(val);
}

/// =========================
//
//  Setters
//
/// =========================

template <class T>
void UnibitNode<T>::setOneNode(UnibitNode* iNewNode) {
    mOneNodePtr = iNewNode;
    // cout<<"setOneNode"<<this->tlm_addr+1<<endl;
    tlmsingelton::getInstance().tlmvarptr->allocate(reinterpret_cast<std::size_t>(iNewNode),this->tlm_addr+1);
    std::size_t tempq=reinterpret_cast<std::size_t>(iNewNode);
    // cout<<"SetOneNodeDiff: "<<reinterpret_cast<std::size_t>(iNewNode)<<" -- "<<mOneNodePtr<<" -- "<<reinterpret_cast<UnibitNode<T>*>(tempq)<<" @tlmaddr:"<<this->tlm_addr+1<<endl;
}

template <class T>
void UnibitNode<T>::setZeroNode(UnibitNode* iNewNode) {
    mZeroNodePtr = iNewNode;
    // cout<<"setZeroNode"<<this->tlm_addr+2<<endl;
    tlmsingelton::getInstance().tlmvarptr->allocate(reinterpret_cast<std::size_t>(iNewNode),this->tlm_addr+2);

}

template <class T>
void UnibitNode<T>::setAction(T iNewAction, int iActionSize, bool iActionFlag) {
    // cout<<"~~setAction"<<this<<endl;
    T temp = iNewAction;
    // tlmsingelton::getInstance().tlmvarptr->allocate(static_cast<std::size_t>(iFlag),this->tlm_addr+3);
    mAction = temp;
    mActionSize = iActionSize;
    mActionFlag = iActionFlag;
    tlmsingelton::getInstance().tlmvarptr->allocate(static_cast<std::size_t>(mActionFlag),this->tlm_addr+3);
}

template <class T>
void UnibitNode<T>::setActionFlag(bool iFlag) {
    mActionFlag = iFlag;
    // cout<<"setActionFlag"<<this->tlm_addr+2<<endl;
    tlmsingelton::getInstance().tlmvarptr->allocate(static_cast<std::size_t>(iFlag),this->tlm_addr+3);

}

template <class T>
void* UnibitNode<T>::operator new(long unsigned size) throw(const char*) {
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
void* UnibitNode<T>::operator new[](long unsigned size) throw(const char*) {
    cout << "   Node::operator new[](" << size << ")\t";
    void *p = malloc(size); if (!p) throw "Node::operator new[]() error";
    cout << static_cast<void*>(p) << endl;
    return p;
}
template <class T>
void UnibitNode<T>::operator delete(void *p) {
    UnibitNode *fp = static_cast<UnibitNode*>(p);
#if debug_node_delete
    cout << "   Node::operator delete(" << p << "):\t" << endl;
#endif
  free(p);
}
template <class T>
void UnibitNode<T>::operator delete[](void *p) {
#if debug_node_delete
    cout << "   Node::operator delete[](" << p << ")" << endl;
#endif
    free(p);
}

#endif /* defined(__UnibitNode__) */
