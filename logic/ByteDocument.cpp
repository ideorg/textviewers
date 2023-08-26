//
// Created by andrzej on 8/23/23.
//

#include <cassert>
#include "ByteDocument.h"
#include "UTF/UTF.hpp"

using namespace std;
using namespace vl;

bool ByteDocument::isNewlineChar(const char c) {
    return c == '\r' || c == '\n';
}

int64_t ByteDocument::searchEndOfLine(int64_t startOffset) {
    if (m_maxLineLen) {
        int64_t offset = startOffset;
        int64_t possibleBreakAt = (offset / m_maxLineLen) * m_maxLineLen + m_maxLineLen;
        int64_t possibleBreakCorrected = correctPossibleBreak(possibleBreakAt);
        while (offset < m_fileSize && !isNewlineChar(m_addr[offset]) && offset < possibleBreakCorrected)
            offset++;
        assert(offset == m_fileSize || isNewlineChar(m_addr[offset]) || offset == possibleBreakCorrected);
        if (offset == possibleBreakCorrected && isFirstChunkStart(startOffset)) {
            possibleBreakAt += m_maxLineLen;
            possibleBreakCorrected = correctPossibleBreak(possibleBreakAt);
            while (offset < m_fileSize && !isNewlineChar(m_addr[offset]) && offset < possibleBreakCorrected)
                offset++;
        }
        assert(offset == m_fileSize || isNewlineChar(m_addr[offset]) || offset == possibleBreakCorrected);
        return offset;
    } else {
        int64_t offset = startOffset;
        while (offset < m_fileSize && !isNewlineChar(m_addr[offset]))
            offset++;
        assert(offset == m_fileSize || isNewlineChar(m_addr[offset]));
        return offset;
    }
}

int64_t ByteDocument::correctPossibleBreak(int64_t possibleBreakAt) {
    if (possibleBreakAt < m_BOMsize || possibleBreakAt >= m_fileSize)
        return possibleBreakAt;
    if (m_addr[possibleBreakAt] == '\n' && possibleBreakAt > m_BOMsize && m_addr[possibleBreakAt - 1] == '\r')
        return possibleBreakAt + 1;
    UTF utf;
    return utf.findNextUtf8OrTheSame(m_addr + possibleBreakAt,
                                     m_addr + std::max((int64_t) m_BOMsize, possibleBreakAt - (UTF::MAXCHARLEN - 1)),
                                     m_addr + m_fileSize) - m_addr;
}

bool ByteDocument::isFirstChunkStart(int64_t offset) {
    return offset <= m_BOMsize || isNewlineChar(m_addr[offset - 1]);
}

ByteDocument::ByteDocument(const char *addr, int64_t fileSize, int64_t maxLineLen) :
        m_addr(addr), m_fileSize(fileSize), m_maxLineLen(maxLineLen) {
}

int64_t ByteDocument::skipLineBreakEx(int64_t eolPos, int64_t len) {
    int64_t next = skipLineBreak(eolPos);
    assert(next<=m_fileSize);
    if (next == m_fileSize && len>0)
        return eolPos;
    else
        return next;
}

int64_t ByteDocument::skipLineBreak(int64_t pos) {
    assert(pos <= m_fileSize);
    if (pos == m_fileSize) return pos;
    const char c = m_addr[pos];
    assert(pos <= m_fileSize - 1);
    if (c == '\r' && pos + 1 <= m_fileSize - 1 && m_addr[pos + 1] == '\n')
        return pos + 2;
    else if (isNewlineChar(c))
        return pos + 1;
    else //MaxLine break
        return pos;
}

int64_t ByteDocument::firstOfCRLF(int64_t position) {
    assert(position>=m_BOMsize && position < m_fileSize && isNewlineChar(m_addr[position]));
    char c = m_addr[position];
    if (c == '\r')
        return position;
    else {
        if (position == m_BOMsize)
            return position;
        else {
            char b = m_addr[position - 1];
            if (b == '\r')
                return position - 1;
            else
                return position;
        }
    }
}

bool ByteDocument::empty() {
    assert(m_fileSize >= m_BOMsize);
    return m_fileSize == m_BOMsize;
}

int64_t ByteDocument::gotoBeginLine(int64_t offset, ByteDocument::EndLine maybeInside) {
    assert(offset <= m_fileSize);
    if (offset == m_fileSize) return m_fileSize;
    if (isNewlineChar(m_addr[offset])) {
        offset = firstOfCRLF(offset);
        if (lineIsEmpty(offset))
            return offset;
        offset--;
    }
    return gotoBeginNonEmptyLine(offset, maybeInside);
}

bool ByteDocument::lineIsEmpty(int64_t offset) {
    assert(offset < m_fileSize && isNewlineChar(m_addr[offset]));
    if (offset == m_fileSize - 1)
        return true;
    if (offset == m_fileSize - 2 && m_addr[offset] == '\r' && m_addr[offset + 1] == '\n')
        return true;
    assert(offset >= m_BOMsize);
    return offset == m_BOMsize || isNewlineChar(m_addr[offset - 1]);
}

int64_t ByteDocument::gotoBeginNonEmptyLine(int64_t start, ByteDocument::EndLine maybeInside) {
    assert(start >= m_BOMsize);
    assert(start < m_fileSize && !isNewlineChar(m_addr[start]));
    int64_t possibleBreakAt = (start / m_maxLineLen) * m_maxLineLen;
    int64_t possibleBreakCorrected = correctPossibleBreak(possibleBreakAt);
    if (possibleBreakCorrected > start && possibleBreakAt >= m_maxLineLen) {
        possibleBreakAt -= m_maxLineLen;
        possibleBreakCorrected = correctPossibleBreak(possibleBreakAt);
    }
    int64_t offset = start;
    while (offset > m_BOMsize && !isNewlineChar(m_addr[offset - 1])) {
        if (offset == possibleBreakCorrected) {
            if (!isFirstChunkInside(offset)) {
                return offset;
            }
        }
        offset--;
    }
    return offset;
}

bool ByteDocument::isFirstChunkInside(int64_t offset) {
    return !startInsideSegment(offset - m_maxLineLen);
}

bool ByteDocument::startInsideSegment(int64_t offset) {
    if (offset <= m_BOMsize + UTF::MAXCHARLEN - 1) return false;
    int64_t nSegment = offset / m_maxLineLen;
    int64_t start = (max((int64_t) m_BOMsize + 1, nSegment * m_maxLineLen)) - 1;
    int64_t end = (nSegment * m_maxLineLen + m_maxLineLen) - 1;
    for (int64_t pos = start; pos < end; pos++)
        if (isNewlineChar(m_addr[pos])) {
            return false;
        }
    return true;
}

std::optional<LinePoints> ByteDocument::firstLine() {
    if (fileIsEmpty())
        return nullopt;
    LinePoints lp;
    lp.offset = m_BOMsize;
    lp.len = searchEndOfLine(m_BOMsize) - m_BOMsize;
    lp.fullLen = skipLineBreakEx(lp.offset + lp.len, lp.len);
    return make_optional(lp);
}

std::optional<LinePoints> ByteDocument::lastLine() {
    if (fileIsEmpty())
        return nullopt;
    LinePoints lp;
    auto eolPos = firstOfCRLF(m_fileSize - 1);
    lp.offset = gotoBeginLine(eolPos, elTrueEol);
    lp.len = eolPos - lp.offset;
    lp.fullLen = m_fileSize - lp.offset;
    return make_optional(lp);
}

LinePoints ByteDocument::lineEnclosing(int64_t position) {
    assert(m_fileSize > m_BOMsize);
    LinePoints lp;
    lp.offset = gotoBeginLine(position, elMaybeInside);
    auto eolPos = searchEndOfLine(position);
    lp.len = eolPos - lp.offset;
    auto next = skipLineBreakEx(eolPos, lp.len);
    lp.fullLen = next - lp.offset;
    return lp;
}

std::string_view ByteDocument::line(const LinePoints &linePoints) {
    std::string_view view(linePoints.offset + m_addr, linePoints.len);
    return view;
}

std::optional<LinePoints> ByteDocument::lineBefore(const LinePoints &linePoints) {
    if (isFirstInFile(linePoints))
        return nullopt;
    LinePoints lp;
    auto eolPos = firstOfCRLF(linePoints.offset - 1);
    lp.offset = gotoBeginLine(eolPos, elTrueEol);
    lp.len = eolPos - lp.offset;
    lp.fullLen = linePoints.offset - lp.offset;
    return make_optional(lp);
}

std::optional<LinePoints> ByteDocument::lineAfter(const LinePoints &linePoints) {
    if (isLastInFile(linePoints))
        return nullopt;
    LinePoints lp;
    lp.offset = linePoints.offset + linePoints.fullLen;
    lp.len = searchEndOfLine(lp.offset) - lp.offset;
    lp.fullLen = skipLineBreakEx(lp.offset + lp.len, lp.len);
    return make_optional(lp);
}

bool ByteDocument::isFirstInFile(const LinePoints &linePoints) {
    assert(m_fileSize >= m_BOMsize);
    assert(linePoints.offset >= m_BOMsize);
    return linePoints.offset == m_BOMsize;
}

bool ByteDocument::isLastInFile(const LinePoints &linePoints) {
    assert(m_fileSize >= m_BOMsize);
    assert(linePoints.offset + linePoints.fullLen <= m_fileSize);
    return linePoints.offset + linePoints.fullLen == m_fileSize;
}

bool ByteDocument::fileIsEmpty() {
    assert(m_fileSize >= m_BOMsize);
    return m_fileSize == m_BOMsize;
}