//
// Created by andrzej on 8/23/23.
//

#include <cassert>
#include <algorithm>
#include "Wrap.h"
#include "AbstractView.h"
#include "DString.h"

using namespace vl;
using namespace std;

std::vector<int64_t> Wrap::wrapEnds(std::string_view lineView) {
    std::vector<int64_t> vec;
    if (view->wrapMode() == 0)
        vec.push_back(lineView.size());
    else {
        const char *s = lineView.cbegin();
        const char *seol = lineView.cend();
        if (lineView.length() == 0)
            vec.push_back(0);
        else
            while (s < seol) {
                u32string wrapLine = DString::substr(string_view(s, seol - s), 0, view->screenLineLen());
                s += DString::utf8len(wrapLine);
                vec.push_back(s - lineView.cbegin());
            }
    }
    return vec;
}

int Wrap::find(vector<int64_t> &v, int64_t pos) {
    return upper_bound(v.begin(), v.end(), pos)-v.begin();
}
