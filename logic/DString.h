//
// Created by andrzej on 8/24/23.
//

#ifndef VIEWER_DSTRING_H
#define VIEWER_DSTRING_H


#include "UTF/UTF.hpp"

class DString {
public:
    static std::u32string substr(std::string_view sv, int start, int len);
};


#endif //VIEWER_DSTRING_H
