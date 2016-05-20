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

#ifndef BEHAVIOURAL_COMMON_FREEMEMORYTRACKER_H_
#define BEHAVIOURAL_COMMON_FREEMEMORYTRACKER_H_
#include <iostream>
#include <map>
#include <vector>
#include <utility>
#include <algorithm>
#include <cassert>

class FreeMemoryTracker {
 public:
  typedef uint64_t addr;
  FreeMemoryTracker() = default;
  ~FreeMemoryTracker() = default;

  explicit FreeMemoryTracker(addr start_addr, addr MaximumByteSize) {
    FreeRegions.push_back(std::make_pair(start_addr, MaximumByteSize));
  }

  void init(addr start_addr, addr MaximumByteSize) {
    FreeRegions.push_back(std::make_pair(start_addr, MaximumByteSize));
  }

  void print() {
    std::cout << "-- Printing List --" << std::endl;
    for (auto it = FreeRegions.begin();
       it != FreeRegions.end();
       ++it) {
      std::cout << it->first << " - " << it->second << std::endl;
    }
    std::cout << "-- End Print List --" << std::endl;
    return;
  }
  addr CalcuateRegionSize(std::pair<addr, addr> Region) {
    return (Region.second+1 - Region.first);
  }

  addr allocate(addr bytes_to_allocate) {
    if (!FreeRegions.empty()) {
      // Iterate over Free regions to allocate
      for (auto it = FreeRegions.begin(); it != FreeRegions.end(); it++) {
        auto RegionSize = CalcuateRegionSize(*it);
        if (bytes_to_allocate <= RegionSize) {
          auto AllocatedAddress = it->first;
          // it->first = AllocatedAddress+bytes_to_allocate;
          it->first = AllocatedAddress+1;
          // if (bytes_to_allocate == RegionSize) {
            // FreeRegions.erase(it);
          if (it->first >= it->second) {
            FreeRegions.erase(it);
            cout << "Freeing"<< endl;
          }
          return AllocatedAddress;
        }
      }
      std::cerr << "Ran out of free regions - Cant Allocate" << std::endl;
      std::cerr << "Free regions defrag is also not implemented" << std::endl;
      exit(-1);
    } else {
      std::cerr<< "No more Free Addreesses" <<std::endl;
      print();
      exit(-1);
    }
  }

  bool deallocate(addr Address_to_deallocate, addr bytes_to_deallocate) {
    if (bytes_to_deallocate != 0) {
      FreeRegions.push_back(std::make_pair(Address_to_deallocate, Address_to_deallocate+bytes_to_deallocate-1));  // NOLINT(whitespace/line_length)
      SortFreeRegions();
      return true;
    } else {
      std::cerr << "I can't deallocate "<< bytes_to_deallocate
                << " bytes at:" << Address_to_deallocate << std::endl;
      assert(false);
    }
  }

  void SortFreeRegions() {
    std::sort(
      FreeRegions.begin(),
      FreeRegions.end(),
      [](const std::pair<addr, addr>& firstElem, const std::pair<addr, addr>& secondElem){ return firstElem.first < secondElem.first; });  // NOLINT(whitespace/line_length)
  }


  std::vector<std::pair<addr, addr>> FreeRegions;
  addr MaxRange;
};

#endif  // BEHAVIOURAL_COMMON_FREEMEMORYTRACKER_H_
