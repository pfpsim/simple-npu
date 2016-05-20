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
 * pool_allocator_defines.h
 *
 *  Created on: June 2, 2015
 *      Author: Lemniscate Snickets
 */


#define use_pool_allocator true //This flag is useless right now TODO: add the #if defines for normal Stack type allocator
#define grow_pool_flag true;
#define debug_free_list false
#define debug_pool false
#define pool_size 4096 // 4kb for multi bit tree
#define slot_size 32 //32bytes this means we will have 4096/32 = 128 slots of 32 bytes which will be aligned @ 4bytes/32bits
#define boundry_alignment 4 //4bytes
#define offset 0
