//
// Created by andrzej on 8/24/23.
//

#ifndef VIEWER_DSTRING_H
#define VIEWER_DSTRING_H


#include "UTF/UTF.hpp"

class DString {
public:
    static dstring substr(const std::string_view sv, int start, int len);
};


#endif //VIEWER_DSTRING_H
