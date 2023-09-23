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
    const char *searchWrapWords(const char *s, const char *seol, const char *sprev, std::u32string &dstr);
    uint32_t codeClass(char32_t c);
    int clLastWidth(const std::u32string &dstr, uint32_t cl);
    int clNextWidth(const char *s, const char *seol, uint32_t cl);
};
}

#endif //VIEWER_WRAP_H
