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
//  BitString.cpp
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-09-10.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#include "BitString.h"
#include <cmath>
#include "systemc.h"

/// ==============================================
//
//  Default Constructor
//
/// ==============================================

BitString::BitString() {}

/// ==============================================
//
//  Copy Constructor
//
/// ==============================================

BitString::BitString(const BitString& copy) {
    data = copy.getVector();
}

/// ==============================================
//
//  String Constructor
//
/// ==============================================

BitString::BitString(const std::string& arg) {
    *this = this->operator=(arg);
}

/// ==============================================
//
//  char* Constructor
//
/// ==============================================

BitString::BitString(const char* arg) {
    *this = this->operator=(arg);
}

/// ==============================================
//
//  uint8_t Constructor
//
/// ==============================================

BitString::BitString(const uint8_t* prefix, const int width) {
    for (int i = 0; i < (width + 7)/8; i++) { // TODO: +7 ??? Confirm this with Gordon
        int element = (int)prefix[i];
        (*this) = (*this) + BitString::intToBitString(element, 8);
    }

}

/// =============================================================================
//
//  Range Constructor
//
//  Constructs BitString by copying a given vector from iterator first to last
//
/// ==============================================================================

BitString::BitString(std::vector<bool>::const_iterator first, std::vector<bool>::const_iterator last) : data(first, last) {}

/// ==============================================
//
//  Operator Overloading
//
/// ==============================================

BitString& BitString::operator=(const std::string& arg){
    for (int i = 0; i < arg.size(); i++) {
        if (arg[i] == '1') {
            data.push_back(1);
        } else {
            data.push_back(0);
        }
    }
    wait(1,SC_NS);

    return *this;
}

BitString& BitString::operator=(const char* arg) {
    std::string s = arg;
    wait(1,SC_NS);
    return this->operator=(s);
}

BitString BitString::operator+(const BitString& arg) {
    BitString out = *this;

    for (int i = 0; i < arg.size(); i++) {
        out.push_back(arg[i]);
    }
    wait(1,SC_NS);

    return out;
}

BitString BitString::operator+=(const BitString& arg) {
    for (int i = 0; i < arg.size(); i++) {
        data.push_back(arg[i]);
    }
    wait(1,SC_NS);

    return *this;
}

BitString BitString::operator+=(const char& arg) {
    if (arg == '0') {
        data.push_back(0);
    } else if (arg == '1') {
        data.push_back(1);
    }
    wait(1,SC_NS);

    return *this;
}

bool BitString::operator==(const std::string& arg) {
    wait(1,SC_NS);

    if (arg.size() == 0) {
        if (data.empty()) {
            return true;
        } else {
            return false;
        }
    }

    bool match = true;
    for (int i = 0; i < arg.size(); i++) {
        if (arg[i] == '1') {
            if (data[i] == 0) {
                match = false;
                break;
            }
        } else if (arg[i] == '0') {
            if (data[i] == 1) {
                match = false;
                break;
            }
        } else {
            match = false;
            break;
        }
    }
    return match;
}

bool BitString::operator==(const char* arg) {
    std::string s = arg;
    wait(1,SC_NS);

    return ((*this) == s);
}

bool BitString::operator==(BitString& arg) {
    wait(1,SC_NS);

    return (this->data == arg.data);
}

bool BitString::operator==(const BitString& arg) {
    wait(1,SC_NS);

    return (this->data == arg.data);
}

bool BitString::operator!=(const std::string& arg) {
    wait(1,SC_NS);

    return !((*this) == arg);
}

bool BitString::operator!=(const char* arg) {
    wait(1,SC_NS);

    return !((*this) == arg);
}

bool BitString::operator!=(BitString& arg) {
    wait(1,SC_NS);

    return !((*this) == arg);
}

std::vector<bool>::reference BitString::operator[](std::size_t idx) {
    wait(1,SC_NS);

    return data[idx];
}

std::vector<bool>::const_reference BitString::operator[](std::size_t idx) const {
    wait(1,SC_NS);

    return data[idx];
}

/// =============================================================
//
//  Converts BitString to unsigned int (Decimal representation)
//
/// =============================================================

unsigned int BitString::toInt() const {
    if (data.empty()) {
        return -1;
    }
    return accumulate(data.begin(), data.end(), 0, [](unsigned int x, unsigned int y) { return (x << 1) + y; });
}

/// ==============================================
//
//  Converts BitString to std::string
//
/// ==============================================

std::string BitString::toString() const {
    if (data.empty()) {
        return "";
    }

    std::string s = "";
    for (int i = 0; i < data.size(); i++) {
        if (data[i] == 1) {
            s.append("1");
        } else {
            s.append("0");
        }
    }

    return s;
}


/// ===========================================================================
//
//  Converts BitString to array of uint_8t with 4 elements
//
//  Each element is a decimal representation of an 8-bit segment of
//  the complete 32-bit ipv4 address (as in the dot notation: e.g. 24.10.0.1)
//
/// ============================================================================

uint8_t* BitString::toUInt8(int *width) const {
    BitString temp;
    int numOfElements = ((int)data.size() + 7)/8;
    int desiredWidth = numOfElements * 8;

    for (int i = 0; i < data.size(); i++) {
        temp.push_back(data[i]);
    }

    for (int i = 0; i < desiredWidth - data.size(); i++) {
        temp.push_back(0);
    }

    uint8_t *out = new uint8_t[numOfElements];

    for (int i = 0; i < numOfElements; i++) {
        std::vector<bool>::const_iterator first = temp.data.begin() + (8*i);
        std::vector<bool>::const_iterator last = first + 8;
        BitString e1(first, last);
        out[i] = e1.toInt();
    }

    (*width) = (int)data.size();

    return out;
}

/// ==============================================
//
//  Size
//
//  Get length of BitString
//
/// ==============================================

int BitString::size() const {
    return (int)data.size();
}

int BitString::length() const {
    return size();
}

/// ==============================================
//
//  push_back
//
//  Add bit to end of BitString
//
/// ==============================================

void BitString::push_back(bool arg) {
    data.push_back(arg);
}

/// ==============================================
//
//  pop_back
//
//  Remove Bit from end of BitString
//
/// ==============================================

void BitString::pop_back() {
    data.pop_back();
}

/// ============================================================
//
//  substr
//
//  Creates a substring of size length starting from startPos
//
/// ============================================================

BitString BitString::substr(int startPos, int length) const {
    BitString out;

    if (startPos >= data.size()) {
        throw "BitString::substr -> out of range";
    } else {

        int outLength = length;

        if ((startPos + length) > data.size() || length == -1) {
            outLength = (int)data.size() - startPos;
        }

        for (int i = startPos; i < startPos + outLength; i++) {
            out.push_back(data[i]);
        }

    return out;
    }
}

BitString BitString::append(const BitString& arg) {
    return ((*this) + arg);
}

/// ============================================================
//
//  compare
//
//  Compares two bitstrings. see string.compare()
//
/// ============================================================

int BitString::compare(const BitString& arg) const {
    return (this->toString()).compare(arg.toString());
}

/// ============================================================
//
//  find
//
//  Finds first occurence of arg in BitString. see find.compare()
//
/// ============================================================

int BitString::find(const BitString& arg) const {
    return (int)(this->toString()).find(arg.toString());
}

/// ==============================================
//
//  Get Underlying Vector Container
//
/// ==============================================

std::vector<bool> BitString::getVector() const {
    return data;
}

/// ==============================================
//
//  Convert a string to BitString
//
/// ==============================================

BitString BitString::stringToBitString(std::string arg) {
    BitString out;
    for (int i = 0; i < arg.size(); i++) {
        if (arg[i] == '1') {
            out.push_back(1);
        } else {
            out.push_back(0);
        }
    }
    return out;
}

/// =========================================================
//
//  Convert an int (decimal representation) to BitString
//
/// =========================================================

BitString BitString::intToBitString(unsigned int arg, int numOfBits) {
    BitString temp;
    for (int i = 0; i < numOfBits; i++) {
        if (arg == 0) {
            temp.push_back(0);
        } else {
            int r = arg % 2;
            if (r == 0) {
                temp.push_back(0);
            } else {
                temp.push_back(1);
            }
            arg = arg/2;
        }
    }

    BitString out;

    for (auto i = temp.data.rbegin(); i != temp.data.rend(); i++) {
        out.push_back(*i);
    }

    return out;
}

/// =========================================================
//
//  Overloading of the << operator
//
//  Allows BitString to be printed to console as a string
//
/// =========================================================

std::ostream& operator<<(std::ostream& os, const BitString& obj) {
    os << obj.toString();
    return os;
}
