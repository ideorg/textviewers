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
    std::vector<int> wrapEnds(std::string_view lineView);
};
}

#endif //VIEWER_WRAP_H
