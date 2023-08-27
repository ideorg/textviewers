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
    int64_t eol = firstOfCRLF(end-1);
    string_view view(m_content.data() + offset, eol-offset);
    return make_optional(view);
}

int LineIndexedDocument::lineCount() {
    return max(wholeIndex.size() - 1, 0UL);
}

LineIndexedDocument::LineIndexedDocument(string_view content, int maxLineLen)
        : ByteDocument(content.data(), content.size(), maxLineLen), m_content(content) {
    createIndex(m_content);
}

void LineIndexedDocument::createIndex(std::string_view source) {
    int source_size = (int) source.size();
    if (source_size != source.size())
        throw runtime_error("file too large");
    int position = m_BOMsize;
    while (position < source_size) {
        wholeIndex.push_back(position);
        int eolPos = searchEndOfLineFromStart(position);
        int len = eolPos - position;
        position = skipLineBreak(eolPos);
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
