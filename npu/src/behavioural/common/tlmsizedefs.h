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

#ifndef BEHAVIOURAL_COMMON_TLMSIZEDEFS_H_
#define BEHAVIOURAL_COMMON_TLMSIZEDEFS_H_

#define tlmsize_BitsinaByte 8

#define tlmsize_char 1  // SHOULD ALWAYS BE 1 byte to conform to C++ standard

#ifndef TARGET_POINTER_SIZE
  #define TARGET_POINTER_SIZE sizeof(uintptr_t)  // Change in P4.cpp
#endif
/*
Set sizes according to your target specification
From the spec:
      Besides the minimal bit counts, the C++ Standard guarantees that
      1 == sizeof(char) <= sizeof(short) <= sizeof(int) <= sizeof(long) <= sizeof(long long).
*/

#define tlm_size_bool 1 /* 1 byte */

#define tlmsize_short_int 16/tlmsize_BitsinaByte
#define tlmsize_int 16/tlmsize_BitsinaByte
#define tlmsize_unsigned_short_int 16/tlmsize_BitsinaByte
#define tlmsize_unsigned_int 16/tlmsize_BitsinaByte
#define tlmsize_long_int 32/tlmsize_BitsinaByte
#define tlmsize_unsigned_long_int 32/tlmsize_BitsinaByte
#define tlmsize_long_long_int 64/tlmsize_BitsinaByte
#define tlmsize_unsigned_long_long_int 64/tlmsize_BitsinaByte

#define tlmsize_short tlmsize_short_int
#define tlmsize_signed_short tlmsize_short_int
#define tlmsize_signed_short_int tlmsize_short_int
#define tlmsize_unsigned_short unsigned_short_int
#define tlmsize_signed tlmsize_int
#define tlmsize_signed_int tlmsize_int
#define tlmsize_unsigned tlmsize_unsigned_int
#define tlmsize_long tlmsize_long_int
#define tlmsize_signed_long tlmsize_long_int
#define tlmsize_signed_long_int tlmsize_long_int
#define tlmsize_unsigned_long tlmsize_unsigned_long_int
#define tlmsize_long_long tlmsize_long_long_int
#define tlmsize_signed_long_long tlmsize_long_long_int
#define tlmsize_signed_long_long_int tlmsize_long_long_int
#define tlmsize_unsigned_long_long tlmsize_unsigned_long_long_int

/*signed integer type*/
#define tlmsize_int8_t 8/tlmsize_BitsinaByte
#define tlmsize_int16_t 16/tlmsize_BitsinaByte
#define tlmsize_int32_t 32/tlmsize_BitsinaByte
#define tlmsize_int64_t 64/tlmsize_BitsinaByte

#define tlmsize_int_fast8_t  tlmsize_int8_t
#define tlmsize_int_fast16_t tlmsize_int16_t
#define tlmsize_int_fast32_t tlmsize_int32_t
#define tlmsize_int_fast64_t tlmsize_int64_t

#define tlmsize_int_least8_t  tlmsize_int8_t
#define tlmsize_int_least16_t tlmsize_int16_t
#define tlmsize_int_least32_t tlmsize_int32_t
#define tlmsize_int_least64_t tlmsize_int64_t

#define tlmsize_intmax_t INTMAX_MAX
#define tlmsize_intptr_t TARGET_POINTER_SIZE  // optional

/*unsigned integer type*/
#define tlmsize_uint8_t   8/tlmsize_BitsinaByte
#define tlmsize_uint16_t  16/tlmsize_BitsinaByte
#define tlmsize_uint32_t  32/tlmsize_BitsinaByte
#define tlmsize_uint64_t  64/tlmsize_BitsinaByte

#define tlmsize_uint_fast8_t  tlmsize_uint8_t
#define tlmsize_uint_fast16_t tlmsize_uint16_t
#define tlmsize_uint_fast32_t tlmsize_uint32_t
#define tlmsize_uint_fast64_t tlmsize_uint64_t

#define tlmsize_uint_least8_t  tlmsize_uint8_t
#define tlmsize_uint_least16_t tlmsize_uint16_t
#define tlmsize_uint_least32_t tlmsize_uint32_t
#define tlmsize_uint_least64_t tlmsize_uint64_t

#define tlmsize_uintmax_t UINTMAX_MAX
#define tlmsize_uintptr_t TARGET_POINTER_SIZE  // optional
#endif  // BEHAVIOURAL_COMMON_TLMSIZEDEFS_H_
