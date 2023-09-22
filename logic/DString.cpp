//
// Created by andrzej on 8/24/23.
//

#include "DString.h"
#include "UTF/UTF.hpp"

using namespace std;

u32string DString::substr(const std::string_view sv, int start, int len) {
    UTF utf;
    return utf.u8to32substr(sv,start,len);
}

int64_t DString::utf8len(std::u32string str) {
    UTF utf;
    return utf.getU8Len(str);
}
