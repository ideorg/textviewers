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

int Wrap::fillDString(const char *&s, const char *seol, std::u32string &dstr) {
    UTF utf;
    int width = 0;
    while (s<seol && width < view->screenLineLen()) {
        const char *end;
        dstr[width] = utf.one8to32(s, seol, &end);
        s = end;
        width++;
    }
    return width;
}

std::vector<int64_t> Wrap::wrapEnds(std::string_view lineView) {
    std::vector<int64_t> vec;
    if (view->wrapMode() == 0 || lineView.empty())
        vec.push_back(lineView.size());
    else {
        const char *s = lineView.cbegin();
        const char *seol = lineView.cend();
        u32string wrapLine;
        wrapLine.resize(view->screenLineLen());
        while (s < seol) {
            fillDString(s, seol, wrapLine);
            vec.push_back(s - lineView.cbegin());
        }
    }
    return vec;
}

int Wrap::find(vector<int64_t> &v, int64_t pos) {
    return upper_bound(v.begin(), v.end(), pos)-v.begin();
}
