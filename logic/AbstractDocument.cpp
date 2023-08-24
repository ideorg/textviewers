//
// Created by andrzej on 8/23/23.
//

#include <cassert>
#include "AbstractDocument.h"
#include "UTF/UTF.hpp"

using namespace std;

bool AbstractDocument::isNewlineChar(const char c) {
    return c == '\r' || c == '\n';
}

int64_t AbstractDocument::searchEndOfLine(int64_t startOffset) {
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

int64_t AbstractDocument::correctPossibleBreak(int64_t possibleBreakAt) {
    if (possibleBreakAt < m_BOMsize || possibleBreakAt >= m_fileSize)
        return possibleBreakAt;
    if (m_addr[possibleBreakAt] == '\n' && possibleBreakAt > m_BOMsize && m_addr[possibleBreakAt - 1] == '\r')
        return possibleBreakAt + 1;
    UTF utf;
    return utf.findNextUtf8OrTheSame(m_addr + possibleBreakAt,
                                     m_addr + std::max((int64_t) m_BOMsize, possibleBreakAt - (UTF::MAXCHARLEN - 1)),
                                     m_addr + m_fileSize) - m_addr;
}

bool AbstractDocument::isFirstChunkStart(int64_t offset) {
    return offset <= m_BOMsize || isNewlineChar(m_addr[offset - 1]);
}

AbstractDocument::AbstractDocument(const char *addr, int64_t fileSize, int64_t maxLineLen) :
        m_addr(addr), m_fileSize(fileSize), m_maxLineLen(maxLineLen) {
}

int64_t AbstractDocument::skipLineBreakEx(int64_t eolPos, int64_t len) {
    int64_t next = skipLineBreak(eolPos);
    assert(next<=m_fileSize);
    if (next == m_fileSize && len>0)
        return eolPos;
    else
        return next;
}

int64_t AbstractDocument::skipLineBreak(int64_t pos) {
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

int64_t AbstractDocument::firstOfCRLF(int64_t position) {
    assert(position < m_fileSize && isNewlineChar(m_addr[position]));
    char c = m_addr[position];
    if (c == '\r')
        return position;
    else {
        if (position <= m_BOMsize)
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

bool AbstractDocument::empty() {
    assert(m_fileSize >= m_BOMsize);
    return m_fileSize == m_BOMsize;
}

int64_t AbstractDocument::gotoBeginLine(int64_t offset, AbstractDocument::EndLine maybeInside) {
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

bool AbstractDocument::lineIsEmpty(int64_t offset) {
    assert(offset < m_fileSize && isNewlineChar(m_addr[offset]));
    if (offset == m_fileSize - 1)
        return true;
    if (offset == m_fileSize - 2 && m_addr[offset] == '\r' && m_addr[offset + 1] == '\n')
        return true;
    assert(offset >= m_BOMsize);
    return offset == m_BOMsize || isNewlineChar(m_addr[offset - 1]);
}

int64_t AbstractDocument::gotoBeginNonEmptyLine(int64_t start, AbstractDocument::EndLine maybeInside) {
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

bool AbstractDocument::isFirstChunkInside(int64_t offset) {
    return !startInsideSegment(offset - m_maxLineLen);
}

bool AbstractDocument::startInsideSegment(int64_t offset) {
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
