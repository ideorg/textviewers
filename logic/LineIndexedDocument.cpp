//
// Created by andrzej on 8/23/23.
//

#include "LineIndexedDocument.h"
#include <utility>
#include <cassert>
#include <stdexcept>

using namespace std;
using namespace vl;

optional<string_view> LineIndexedDocument::line(int n) {
    if (n < 0 || n  >= lineCount())
        return nullopt;;
    int64_t offset = wholeIndex[n];
    int64_t end = wholeIndex[n+1];
    int64_t eol;
    if (isNewlineChar(m_addr[end-1]))
        eol = firstOfCRLF(end-1);
    else
        eol = end;
    string_view view(m_content.c_str() + offset, eol-offset);
    return make_optional(view);
}

int LineIndexedDocument::lineCount() {
    return max(wholeIndex.size() - 1, 0UL);
}

LineIndexedDocument::LineIndexedDocument(string content, int maxLineLen)
        : ByteDocument(content.c_str(), content.size(), maxLineLen), m_content(std::move(content)) {
    createIndex(m_content);
}

void LineIndexedDocument::createIndex(std::string source) {
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

bool LineIndexedDocument::linesAreEmpty() {
    return lineCount() == 0;
}

bool LineIndexedDocument::isFirstInFile(int n) {
    assert(n >= 0);
    assert(n < lineCount());
    assert(lineCount() > 0);
    return n == 0;
}

bool LineIndexedDocument::isLastInFile(int n) {
    assert(n >= 0);
    assert(n < lineCount());
    assert(lineCount() > 0);
    return n +1 == lineCount();
}
