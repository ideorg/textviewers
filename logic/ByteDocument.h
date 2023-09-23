//
// Created by andrzej on 8/23/23.
//

#ifndef VIEWER_ABSTRACTDOCUMENT_H
#define VIEWER_ABSTRACTDOCUMENT_H

#include <cstdint>
#include "IByteAccess.h"
#include "gtest/gtest.h"

namespace vl {

class ByteDocument: public virtual IByteAccess {
    FRIEND_TEST(ByteDocument, lineIsEmpty);
    FRIEND_TEST(ByteDocument, forward);
    FRIEND_TEST(ByteDocument, backward);
    FRIEND_TEST(ByteDocumentML, lineIsEmpty);
    FRIEND_TEST(ByteDocumentML, forward);
    FRIEND_TEST(ByteDocumentML, backward);
    FRIEND_TEST(ByteDocumentMLunicode, forward);
    FRIEND_TEST(ByteDocumentMLunicode, backward);
    int64_t correctPossibleBreak(int64_t possibleBreakAt);
    bool skipPossibleBreakForward(int64_t startOffset, int64_t possibleBreakAt);
    bool skipPossibleBreakBackward(int64_t possibleBreakAt);
    const char *m_addr;
    int64_t m_fileSize;
    int64_t m_maxLineLen;
    int m_BOMsize = 0;
    enum EndLine {
        elMaybeInside, elTrueEol
    };//todo really needed?
    bool lineIsEmpty(int64_t offset);
    //forward
    int64_t searchEndOfLine(int64_t startOffset);
    int64_t searchEndOfLineFromStart(int64_t startOffset);
    int64_t skipLineBreak(int64_t pos);
    //backward
    int64_t firstOfCRLF(int64_t position);
    int64_t gotoBeginLine(int64_t offset, EndLine maybeInside);
    int64_t gotoBeginNonEmptyLine(int64_t start, EndLine maybeInside);
public:
    ByteDocument(const char *addr, int64_t fileSize, int64_t maxLineLen = 0);
    int64_t firstByte() override;
    int64_t byteCount() override;
    bool fileIsEmpty() override;
    std::optional<LinePoints> firstLine() override;
    std::optional<LinePoints> lastLine() override;
    LinePoints lineEnclosing(int64_t position) override;
    std::string_view line(const LinePoints &linePoints) override;
    std::string_view line(const SmallLinePoints &linePoints) override;
    std::optional<LinePoints> lineBefore(const LinePoints &linePoints) override;
    std::optional<LinePoints> lineAfter(const LinePoints &linePoints) override;
    bool isFirstInFile(const LinePoints &linePoints) override;
    bool isLastInFile(const LinePoints &linePoints) override;
    static bool isNewlineChar(char c);
};
}

#endif //VIEWER_ABSTRACTDOCUMENT_H
