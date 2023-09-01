//
// Created by andrzej on 8/23/23.
//

#include <cassert>
#include <stdexcept>
#include "ByteDocument.h"
#include "UTF/UTF.hpp"

using namespace std;
using namespace vl;

bool ByteDocument::isNewlineChar(const char c) {
    return c == '\r' || c == '\n';
}

int64_t ByteDocument::searchEndOfLine(int64_t startOffset) {
    if (isNewlineChar(m_addr[startOffset]))
        throw std::runtime_error("searchEndOfLine: startOffset is on a line break");
    if (m_maxLineLen) {
        int64_t offset = startOffset;
        int64_t possibleBreakAt = (offset / m_maxLineLen) * m_maxLineLen + m_maxLineLen;
        int64_t possibleBreakCorrected = correctPossibleBreak(possibleBreakAt);
        while (offset < m_fileSize && !isNewlineChar(m_addr[offset]) && offset < possibleBreakCorrected)
            offset++;
        assert(offset == m_fileSize || isNewlineChar(m_addr[offset]) || offset == possibleBreakCorrected);
        if (skipPossibleBreakForward(startOffset, possibleBreakAt)) {
            possibleBreakAt += m_maxLineLen;
            possibleBreakCorrected = correctPossibleBreak(possibleBreakAt);
            while (offset < m_fileSize && !isNewlineChar(m_addr[offset]) && offset < possibleBreakCorrected)
                offset++;
            assert(offset == m_fileSize || isNewlineChar(m_addr[offset]) || offset == possibleBreakCorrected);
        }
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

ByteDocument::ByteDocument(const char *addr, int64_t fileSize, int64_t maxLineLen) :
        m_addr(addr), m_fileSize(fileSize), m_maxLineLen(maxLineLen) {
    if (m_fileSize >= 3 && m_addr[0] == (char) 0xEF && m_addr[1] == (char) 0xBB && m_addr[2] == (char) 0xBF)
        m_BOMsize = 3;
    else
        m_BOMsize = 0;
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
    assert(position >= m_BOMsize);
    assert(position < m_fileSize);
    if (!isNewlineChar(m_addr[position]))
        throw std::runtime_error("firstOfCRLF: position is not on a line break");
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

int64_t ByteDocument::gotoBeginLine(int64_t offset, ByteDocument::EndLine maybeInside) {
    assert(offset <= m_fileSize);
    if (offset == m_fileSize) offset--;
    if (isNewlineChar(m_addr[offset])) {
        offset = firstOfCRLF(offset);
        if (lineIsEmpty(offset))
            return offset;
        offset--;
    }
    return gotoBeginNonEmptyLine(offset, maybeInside);
}

bool ByteDocument::lineIsEmpty(int64_t offset) {
    assert(offset < m_fileSize);
    if (!isNewlineChar(m_addr[offset]))
        return false;
    offset = firstOfCRLF(offset);
    assert(offset >= m_BOMsize);
    if (offset == m_BOMsize)
        return true;
    auto offset1 = skipLineBreak(offset);
    if (offset1 == m_fileSize)
        return true;
    return isNewlineChar(m_addr[offset - 1]);
}

int64_t ByteDocument::gotoBeginNonEmptyLine(int64_t start, ByteDocument::EndLine maybeInside) {
    assert(start >= m_BOMsize);
    assert(start < m_fileSize);
    if (isNewlineChar(m_addr[start]))
        throw std::runtime_error("gotoBeginNonEmptyLine: start is on a line break");
    if (m_maxLineLen) {
        int64_t possibleBreakAt = (start / m_maxLineLen) * m_maxLineLen;
        int64_t possibleBreakCorrected = correctPossibleBreak(possibleBreakAt);
        if (possibleBreakCorrected > start && possibleBreakAt >= m_maxLineLen) {
            possibleBreakAt -= m_maxLineLen;
            possibleBreakCorrected = correctPossibleBreak(possibleBreakAt);
        }
        int64_t offset = start;
        while (offset > m_BOMsize && !isNewlineChar(m_addr[offset - 1]) && offset > possibleBreakCorrected)
            offset--;
        assert(offset == m_BOMsize || isNewlineChar(m_addr[offset - 1]) || offset == possibleBreakCorrected);
        if (skipPossibleBreakBackward(possibleBreakAt)) {
            possibleBreakAt -= m_maxLineLen;
            possibleBreakCorrected = correctPossibleBreak(possibleBreakAt);
            while (offset > m_BOMsize && !isNewlineChar(m_addr[offset - 1]) && offset > possibleBreakCorrected)
                offset--;
            assert(offset == m_BOMsize || isNewlineChar(m_addr[offset - 1]) || offset == possibleBreakCorrected);
        }
        return offset;
    } else {
        int64_t offset = start;
        while (offset > m_BOMsize && !isNewlineChar(m_addr[offset - 1]))
            offset--;
        return offset;
    }
}

std::optional<LinePoints> ByteDocument::firstLine() {
    if (fileIsEmpty())
        return nullopt;
    LinePoints lp;
    lp.offset = m_BOMsize;
    if (isNewlineChar(m_addr[lp.offset]))
        lp.len = 0;
    else
        lp.len = searchEndOfLine(m_BOMsize) - m_BOMsize;
    lp.fullLen = skipLineBreak(lp.offset + lp.len) - m_BOMsize;
    if (lp.offset + lp.fullLen == m_fileSize && lp.len > 0)
        lp.fullLen = lp.len;
    return make_optional(lp);
}

std::optional<LinePoints> ByteDocument::lastLine() {
    if (fileIsEmpty())
        return nullopt;
    LinePoints lp;
    int64_t eolPos;
    if (isNewlineChar(m_addr[m_fileSize - 1]))
        eolPos = firstOfCRLF(m_fileSize - 1);
    else
        eolPos = m_fileSize;
    lp.offset = gotoBeginLine(eolPos, elTrueEol);
    lp.len = eolPos - lp.offset;
    lp.fullLen = m_fileSize - lp.offset;
    return make_optional(lp);
}

LinePoints ByteDocument::lineEnclosing(int64_t position) {
    assert(m_fileSize > m_BOMsize);
    LinePoints lp;
    lp.offset = gotoBeginLine(position, elMaybeInside);
    int64_t eolPos;
    if (isNewlineChar(m_addr[position]))
        eolPos = firstOfCRLF(position);
    else
        eolPos = searchEndOfLine(position);
    lp.len = eolPos - lp.offset;
    auto next = skipLineBreak(eolPos);
    lp.fullLen = next - lp.offset;
    return lp;
}

std::string_view ByteDocument::line(const LinePoints &linePoints) {
    std::string_view view(linePoints.offset + m_addr, linePoints.len);
    return view;
}

std::string_view ByteDocument::line(const SmallLinePoints &linePoints) {
    std::string_view view(linePoints.offset + m_addr, linePoints.len);
    return view;
}

std::optional<LinePoints> ByteDocument::lineBefore(const LinePoints &linePoints) {
    if (isFirstInFile(linePoints))
        return nullopt;
    LinePoints lp;
    int64_t eolPos;
    if (linePoints.offset + linePoints.fullLen == m_fileSize && linePoints.len == 0) {
        eolPos = linePoints.offset;
        lp.offset = gotoBeginLine(eolPos-1, elTrueEol);
    } else {
        eolPos = firstOfCRLF(linePoints.offset - 1);
        lp.offset = gotoBeginLine(eolPos, elTrueEol);
    }
    lp.len = eolPos - lp.offset;
    lp.fullLen = linePoints.offset - lp.offset;
    return make_optional(lp);
}

std::optional<LinePoints> ByteDocument::lineAfter(const LinePoints &linePoints) {
    if (isLastInFile(linePoints))
        return nullopt;
    LinePoints lp;
    lp.offset = linePoints.offset + linePoints.fullLen;
    lp.len = searchEndOfLineFromStart(lp.offset) - lp.offset;
    lp.fullLen = skipLineBreak(lp.offset + lp.len) - lp.offset;
    if (lp.offset + lp.fullLen == m_fileSize && lp.len > 0)
        lp.fullLen = lp.len;
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

int64_t ByteDocument::byteCount() {
    return m_fileSize;
}

int64_t ByteDocument::firstByte() {
    return m_BOMsize;
}

int64_t ByteDocument::searchEndOfLineFromStart(int64_t startOffset) {
    if (isNewlineChar(m_addr[startOffset]))
        return startOffset;
    else
        return searchEndOfLine(startOffset);
}

bool ByteDocument::skipPossibleBreakForward(int64_t startOffset, int64_t possibleBreakAt) {
    int64_t prev = possibleBreakAt - m_maxLineLen;
    if (prev<m_BOMsize)
        return true;
    int64_t prevCorrected = correctPossibleBreak(prev);
    for (int64_t i = startOffset; i>=prevCorrected; i--) {
        if (i==m_BOMsize || isNewlineChar(m_addr[i-1]))
            return true;
    }
    return false;
}

bool ByteDocument::skipPossibleBreakBackward(int64_t possibleBreakAt) {
    int64_t possibleBreakCorrected = correctPossibleBreak(possibleBreakAt);
    int64_t prev = possibleBreakAt - m_maxLineLen;
    if (prev<m_BOMsize)
        return true;
    int64_t prevCorrected = correctPossibleBreak(prev);
    for (int64_t i = possibleBreakCorrected; i>=prevCorrected; i--) {
        if (i==m_BOMsize || isNewlineChar(m_addr[i-1]))
            return true;
    }
    return false;
}

