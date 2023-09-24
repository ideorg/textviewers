//
// Created by andrzej on 8/23/23.
//

#include "LineIndexedDocument.h"
#include <utility>
#include <cassert>
#include <stdexcept>

using namespace std;
using namespace vl;

optional<string_view> LineIndexedDocument::lineByIndex(int n) {
    if (n < 0 || n  >= lineCount())
        return nullopt;;
    auto slp = wholeIndex[n];
    return byteDocument->line(slp);
}

int LineIndexedDocument::lineCount() {
    return wholeIndex.size();
}

LineIndexedDocument::LineIndexedDocument(string_view content, int maxLineLen)
        : m_content(content) {
    byteDocument = make_unique<ByteDocument>(content.data(), content.size(), maxLineLen);
    createIndex(m_content);
}

void LineIndexedDocument::createIndex(std::string_view source) {
    IByteAccess *idoc = byteDocument.get();
    int source_size = (int) source.size();
    if (source_size != source.size())
        throw runtime_error("file too large");
    auto opt = idoc->firstLine();
    if (!opt)
        return;
    while (opt) {
        auto lp = opt.value();
        SmallLinePoints slp;
        slp.offset = lp.offset;
        slp.len = (int)lp.len;
        wholeIndex.push_back(slp);
        opt = idoc->lineAfter(lp);
    }
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

std::string_view LineIndexedDocument::getBytes(FilePosition from, FilePosition to) {
    int64_t fromGlobalOffset = wholeIndex[from.lineNumber].offset + from.offset;
    int64_t toGlobalOffset = wholeIndex[to.lineNumber].offset + to.offset;
    if (toGlobalOffset<=fromGlobalOffset)
        return {};
    else
        return {byteDocument->ofsetToPointer(fromGlobalOffset), (size_t)(toGlobalOffset-fromGlobalOffset)};
}
