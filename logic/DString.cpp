//
// Created by andrzej on 8/24/23.
//

#include "DString.h"

dstring DString::substr(const std::string_view sv, int start, int len) {
    UTF utf;
    return utf.u8to32substr(sv,start,len);
}
