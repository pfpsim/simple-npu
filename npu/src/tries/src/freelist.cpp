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

#include "freelist.h"

Freelist::Freelist(void* start, void* end, size_t elementSize, size_t alignment, size_t offset){

	union
	{
	  void* as_void;
	  char* as_char;
	  Freelist* as_self;
	};

	// assume as_self points to the first entry in the free list
	m_next = as_self;
	as_char += elementSize;

	// initialize the free list - make every m_next of each element point to the next element in the list
	Freelist* runner = m_next;
	for (size_t i=1; i<elementSize; ++i)
	{
	  runner->m_next = as_self;
	  runner = as_self;
	  as_char += elementSize;
	}

	runner->m_next = nullptr;


}


void* Freelist::Obtain(void)
{
  // is there an entry left?
  if (m_next == nullptr)
  {
    // we are out of entries lets go to timbuktu
    return nullptr;
  }

  // obtain one element from the head of the free list
  Freelist* head = m_next;
  m_next = head->m_next;
  return head;
}

void Freelist::Return(void* ptr)
{
  // put the returned element at the head of the free list
  Freelist* head = static_cast<Freelist*>(ptr);
  head->m_next = m_next;
  m_next = head;
}
