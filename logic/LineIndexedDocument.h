//
// Created by andrzej on 8/23/23.
//

#ifndef VIEWER_LINEDOCUMENT_H
#define VIEWER_LINEDOCUMENT_H

#include <vector>
#include <string>
#include "ILineAccess.h"
#include "ByteDocument.h"

namespace vl {

class LineIndexedDocument : public virtual ByteDocument, public virtual ILineAccess {
    std::string_view m_content;
    std::vector<SmallLinePoints> wholeIndex;
    void createIndex(std::string_view source);
public:
    explicit LineIndexedDocument(std::string_view content, int maxLineLen = 0);
    std::optional<std::string_view> lineByIndex(int n) override;
    int lineCount() override;
    bool linesAreEmpty() override;
    bool isFirstInFile(int n) override;
    bool isLastInFile(int n) override;
};
}

#endif //VIEWER_LINEDOCUMENT_H
