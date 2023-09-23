//
// Created by andrzej on 8/23/23.
//

#ifndef VIEWER_WRAP_H
#define VIEWER_WRAP_H

#include <vector>
#include <string_view>

namespace vl {

class AbstractView;

class Wrap {
    AbstractView *view;
public:
    Wrap(AbstractView *view):view(view){}
    int fillDString(const char *&s, const char *seol, std::u32string &dstring);
    std::vector<int64_t> wrapEnds(std::string_view lineView);
    static int find(std::vector<int64_t> &v, int64_t pos);
};
}

#endif //VIEWER_WRAP_H
