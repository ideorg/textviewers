//
// Created by andrzej on 8/23/23.
//

#include <cassert>
#include <algorithm>
#include "Wrap.h"
#include "AbstractView.h"
#include "DString.h"
#include "ByteDocument.h"

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

const char *Wrap::searchWrapWords(const char *s, const char *seol, const char *sprev, u32string &dstr) {
    uint32_t cl = codeClass(dstr[view->screenLineLen() - 1]);
    if (cl > 0) {
        int countLast = clLastWidth(dstr, cl);
        int countNext = clNextWidth(s, seol, cl);
        if (countNext > 0 && countLast + countNext <= view->screenLineLen()) {
            int64_t actual;
            UTF utf;
            s = utf.backwardNcodes(s, countLast, sprev, actual);
            assert(countLast == actual);
        }
    }
    return s;
}

std::vector<int64_t> Wrap::wrapEnds(std::string_view lineView) {
    std::vector<int64_t> vec;
    if (view->wrapMode() == 0 || lineView.empty())
        vec.push_back(lineView.size());
    else {
        const char *s = lineView.cbegin();
        const char *seol = lineView.cend();
        const char *sprev = s;
        u32string wrapLine;
        wrapLine.resize(view->screenLineLen());
        while (s < seol) {
            int width = fillDString(s, seol, wrapLine);
            if (width == view->screenLineLen() && view->wrapMode() == 2) {
                s = searchWrapWords(s, seol, sprev, wrapLine);
            }
            vec.push_back(s - lineView.cbegin());
            sprev = s;
        }
    }
    return vec;
}

int Wrap::find(vector<int64_t> &v, int64_t pos) {
    return upper_bound(v.begin(), v.end(), pos)-v.begin();
}

uint32_t Wrap::codeClass(char32_t c) {
    if (ByteDocument::isNewlineChar(char(c)))
        return -1;
    if (c == ' ' || c == '\t')
        return 0;
    else if (c == '_' || c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c >= '0' && c <= '9'
             || c >= 0x80 && c < 0x250)
        return 1;
    else if (c < 128)
        return 2;
    else
        return c & 0xffffff00;
}

int Wrap::clLastWidth(const u32string &dstr, uint32_t cl) {
    int countLast = 0;
    for (int k = view->screenLineLen() - 1; k >= 0; k--) {
        if (codeClass(dstr[k]) == cl)
            countLast++;
        else
            break;
    }
    return countLast;
}

int Wrap::clNextWidth(const char *s, const char *seol, uint32_t cl) {
    int countNext = 0;
    const char *s1 = s;
    UTF utf;
    while (s1 < seol && countNext < view->screenLineLen()) {
        const char *end;
        uint d = utf.one8to32(s1, seol, &end);//no need here keep an eye on \t
        s1 = end;
        if (codeClass(d) != cl)
            break;
        countNext++;
        s1++;
    }
    return countNext;
}
