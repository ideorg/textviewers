//
// Created by andrzej on 8/23/23.
//

#include "LineDocument.h"
#include <utility>
#include <cassert>
#include <stdexcept>

using namespace std;
using namespace vl;

optional<string_view> LineDocument::line(int n) {
    if (n < 0 || n + 1 >= wholeIndex.size())
        return nullopt;;
    int offset = wholeIndex[n];
    int end = wholeIndex[n+1];
    int eol = firstOfCRLF(end-1);
    string_view view(m_content.c_str() + offset, eol-offset);
    return make_optional(view);
}

int LineDocument::lineCount() {
    return max(wholeIndex.size() - 1, 0UL);
}

LineDocument::LineDocument(string content, int maxLineLen)
        : AbstractDocument(content.c_str(), content.size(), maxLineLen), m_content(std::move(content)) {
    createIndex(m_content);
}

void LineDocument::createIndex(std::string source) {
    int source_size = (int) source.size();
    if (source_size != source.size())
        throw runtime_error("file too large");
    int position = m_BOMsize;
    while (position < source_size) {
        wholeIndex.push_back(position);
        int eolPos = searchEndOfLine(position);
        int len = eolPos - position;
        position = skipLineBreakEx(eolPos, len);
    }
    assert(position == source_size);
    wholeIndex.push_back(position);
}

bool LineDocument::linesAreEmpty() {
    return wholeIndex.size() == 0;
}

bool LineDocument::isFirstInFile(int n) {
    assert(n >= 0);
    assert(n < wholeIndex.size());
    assert(wholeIndex.size() > 0);
    return n == 0;
}

bool LineDocument::isLastInFile(int n) {
    assert(n >= 0);
    assert(n < wholeIndex.size());
    assert(wholeIndex.size() > 0);
    return n + 1 == wholeIndex.size();
}
