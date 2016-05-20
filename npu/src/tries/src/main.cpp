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

//#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <iostream>

#include"syscmodules.h"

SC_MODULE(Top)
{
   // Initiator *initiator;
	trie_module* trie;
	Memory* mem;
    SC_CTOR(Top)
    {
    	trie = new  trie_module("system");
    	mem    = new Memory   ("memory");
    	trie->socket_t.bind( mem->socket );
    	//S::getInstance().pointer_to_tree=trie;

    }
};


int sc_main(int argc, char *argv[])  {

    //trie_module prefix_tree("system");
    Top top("top");
    sc_start();

    return 0;
}
