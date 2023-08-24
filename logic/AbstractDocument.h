//
// Created by andrzej on 8/23/23.
//

#ifndef VIEWER_ABSTRACTDOCUMENT_H
#define VIEWER_ABSTRACTDOCUMENT_H

#include <cstdint>

namespace vl {

class AbstractDocument {
    int64_t correctPossibleBreak(int64_t possibleBreakAt);
    bool isFirstChunkStart(int64_t offset);
    int64_t skipLineBreak(int64_t pos);
    bool isFirstChunkInside(int64_t offset);
    bool startInsideSegment(int64_t offset);
protected:
    const char *m_addr;
    int64_t m_fileSize;
    int64_t m_maxLineLen;
    int m_BOMsize = 0;
    static bool isNewlineChar(char c);
    int64_t searchEndOfLine(int64_t startOffset);
    int64_t skipLineBreakEx(int64_t eolPos, int64_t len);
    int64_t firstOfCRLF(int64_t position);
    enum EndLine {
        elMaybeInside, elTrueEol
    };//todo really needed?
    int64_t gotoBeginLine(int64_t offset, EndLine maybeInside);
    bool lineIsEmpty(int64_t offset);
    int64_t gotoBeginNonEmptyLine(int64_t start, EndLine maybeInside);
    bool empty();
public:
    AbstractDocument(const char *addr, int64_t fileSize, int64_t maxLineLen = 0);
};
}

#endif //VIEWER_ABSTRACTDOCUMENT_H