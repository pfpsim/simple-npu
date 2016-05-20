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

/*
 * configdebug.h
 *
 *  Created on: May 20, 2015
 *      Author: Lemniscate Snickets
 */

#ifndef CONFIGDEBUG_H_
#define CONFIGDEBUG_H_


#define dotestlarge true

#define mem_globaldebug false

#define track_host_mem true
#define use_tlm_mem true

#if mem_globaldebug

	/*
	 * Auto generated using insertdebugtraces.py
	 */

	/*PrefixTree.cpp*/
	#define debug_treelvl_constructor true
	#define debug_treelvl_insert true
	#define debug_treelvl_remove true

	/*UnibitNode.cpp*/
	#define debug_node_constructor true
	#define debug_whatis_node_struct_base_tlmaddr true
	#define debug_node_copy_constructor true
	#define debug_node_destructor true

	#define debug_node_getOneNode true
	#define debug_node_getZeroNode true
	#define debug_node_getNextHop true
	#define debug_node_setOneNode true
	#define debug_node_setZeroNode true
	#define debug_node_setNextHop true

	#define debug_node_new true
	#define debug_node_delete true

	/*syscmodules.cpp*/
	#define debug_tlm_mem_transaction true

	/*syscmodules.h*/
	#define debug_singleton_tree_ref true

	/*LC Trie*/
	#define debug_node_getBranchFactor true
	#define debug_node_getSkip true
	#define debug_node_getLeftNodePos true
	#define debug_node_setSkip true
	#define debug_node_setBranchFactor true
	#define debug_node_setLeftNodePos true

	/*Hash Tree*/
	#define debug_node_getFlag true
	#define debug_node_getKeyBitmap true
	#define debug_node_getKeyLength true
	#define debug_node_getValuePtr true
	#define debug_node_getValueLength true
	#define debug_node_setFlag true
	#define debug_node_setKeyBitmap true
	#define debug_node_setValuePtr true
	#define debug_node_setKeyLength true
	#define debug_node_setValueLength true

/*MutlbitNodeTree*/
#define debug_node_getStride true
#define debug_node_getLength true
#define debug_node_setStride true

#else

	#define debug_treelvl_constructor false
	#define debug_treelvl_insert false
	#define debug_treelvl_remove false

	#define debug_node_constructor false
	#define debug_whatis_node_struct_base_tlmaddr false
	#define debug_node_copy_constructor false
	#define debug_node_destructor false

	#define debug_node_getOneNode false
	#define debug_node_getZeroNode false
	#define debug_node_getNextHop false
	#define debug_node_setOneNode false
	#define debug_node_setZeroNode false
	#define debug_node_setNextHop false

	#define debug_node_new false
	#define debug_node_delete false

	#define singleton_tree_ref false
	#define debug_tlm_mem_transaction false

	/*LC Trie*/
	#define debug_node_getBranchFactor false
	#define debug_node_getSkip false
	#define debug_node_getLeftNodePos false
	#define debug_node_setSkip false
	#define debug_node_setBranchFactor false
	#define debug_node_setLeftNodePos false

	/*Hash Tree*/
	#define debug_node_getFlag false
	#define debug_node_getKeyBitmap false
	#define debug_node_getKeyLength false
	#define debug_node_getValuePtr false
	#define debug_node_getValueLength false
	#define debug_node_setFlag false
	#define debug_node_setKeyBitmap false
	#define debug_node_setValuePtr false
	#define debug_node_setKeyLength false
	#define debug_node_setValueLength false

	#define debug_node_getStride false
	#define debug_node_getLength false
	#define debug_node_setStride false
#endif


#endif /* CONFIGDEBUG_H_ */
