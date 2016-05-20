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
//  LCNode.cpp
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-02-08.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#include <iostream>
#include "LCNode.h"
#include "tlmsingletoninclude.h"

#include "configdebug.h"
/// ==================================
//
//  Constructor
//
/// ==================================

LCNode::LCNode(int iBranch, int iSkip) : mBranchFactor(iBranch), mSkip(iSkip), mLeftNodePos(0) {

	int addr = tlmsingelton::getInstance().tlmvarptr->allocate_mem(32*3); //Allocate and Construct in tlm memory

	#if debug_whatis_node_struct_base_tlmaddr
	cout	<<"LC Node Constructor"<<endl
			<<" |-Addr in tlm mem	:"<<hex<<addr<<endl;
	#endif

	this->tlm_addr = addr;

	 tlmsingelton::getInstance().tlmvarptr->allocate(static_cast<std::size_t>(iBranch),this->tlm_addr);
	 tlmsingelton::getInstance().tlmvarptr->allocate(static_cast<std::size_t>(iSkip),this->tlm_addr+1);
	 tlmsingelton::getInstance().tlmvarptr->allocate(static_cast<std::size_t>(0),this->tlm_addr+2);
}

/// ==================================
//
//  Destructor
//
/// ==================================

LCNode::~LCNode() {
	#if debug_node_destructor
	cout<<"LC Node Destructor Called"<<endl;
	#endif

}

/// ==================================
//
//  Getters
//
/// ==================================

int LCNode::getBranchFactor() const {

	std::size_t val = tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr);

	#if debug_node_getBranchFactor
		cout<<"Get Branch Factor:"<<endl
			<<" |-TLM val		:"<<static_cast<int>(val)<<endl
			<<" |-BranchFactor	:"<<mBranchFactor<<endl;
	#endif

	return static_cast<int>(val);
    //return mBranchFactor;

}

int LCNode::getSkip() const {
	std::size_t val = tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+1);

	#if debug_node_getSkip
		cout<<"Get Branch Factor:"<<endl
			<<" |-TLM val		:"<<static_cast<int>(val)<<endl
			<<" |-mSkip			:"<<mSkip<<endl;
	#endif

	return static_cast<int>(val);

    //return mSkip;
}

int LCNode::getLeftNodePos() const {

	std::size_t val = tlmsingelton::getInstance().tlmvarptr->read_mem(this->tlm_addr+2);

	#if debug_node_getLeftNodePos
		cout<<"Get LeftNodePos:"<<endl
			<<" |-TLM val		:"<<static_cast<int>(val)<<endl
			<<" |-mLeftNodePos	:"<<mLeftNodePos<<endl;
	#endif

	return static_cast<int>(val);

    //return mLeftNodePos;
}

/// ==================================
//
//  Setters
//
/// ==================================

void LCNode::setBranchFactor(int iBranch) {

	mBranchFactor = iBranch;

	#if debug_node_setBranchFactor
		cout<<"Set Branch Factor"<<endl
			<<" |-Set TLM val			:"<<static_cast<std::size_t>(iBranch)<<endl
			<<" |-Set setBranchFactor	:"<<static_cast<std::size_t>(mBranchFactor)<<endl;
	#endif

	tlmsingelton::getInstance().tlmvarptr->allocate(static_cast<std::size_t>(iBranch),this->tlm_addr);

}

void LCNode::setSkip(int iSkip) {
    mSkip = iSkip;

	#if debug_node_setSkip
		cout<<"Set Skip"<<endl
			<<" |-Set TLM val   :"<<static_cast<std::size_t>(iSkip)<<endl
			<<" |-Set setSkip   :"<<static_cast<std::size_t>(mSkip)<<endl;
	#endif

	tlmsingelton::getInstance().tlmvarptr->allocate(static_cast<std::size_t>(iSkip),this->tlm_addr+1);
}

void LCNode::setLeftNodePos(int iPos) {
    mLeftNodePos = iPos;

	#if debug_node_setLeftNodePos
		cout<<"Set setLeftNodePos"<<endl
			<<" |-Set TLM val			:"<<static_cast<std::size_t>(iPos)<<endl
			<<" |-Set setLeftNodePos	:"<<static_cast<std::size_t>(mLeftNodePos)<<endl;
	#endif

	tlmsingelton::getInstance().tlmvarptr->allocate(static_cast<std::size_t>(iPos),this->tlm_addr+2);

}

/*
 *------------- Allocation Operators ----------------------------------------------
 */

void* LCNode::operator new(long unsigned size) throw(const char*) {

	#if debug_node_new
    cout << "   Node::operator new(" << size << ")\t";
	#endif

    void *p = malloc(size); if (!p) throw "Node::operator new() error";

    #if debug_node_new
    cout << static_cast<void*>(p) << endl;
	#endif

    return p;

}

void* LCNode::operator new[](long unsigned size) throw(const char*) {
    cout << "   Node::operator new[](" << size << ")\t";
    void *p = malloc(size); if (!p) throw "Node::operator new[]() error";
    cout << static_cast<void*>(p) << endl;
    return p;
}
void LCNode::operator delete(void *p) {
    free(p);
}
void LCNode::operator delete[](void *p) {
    cout << "   Node::operator delete[]()" << endl;
    cout << "   Node::operator delete[](" << p << ")" << endl;
    free(p);
}
