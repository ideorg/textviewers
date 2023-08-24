//
// Created by andrzej on 8/23/23.
//

#ifndef VIEWER_LINEDOCUMENT_H
#define VIEWER_LINEDOCUMENT_H

#include <vector>
#include <string>
#include "ILineAccess.h"
#include "AbstractDocument.h"

namespace vl {

class LineDocument : public AbstractDocument, public ILineAccess {
    std::vector<std::string> stringList;
    std::string m_content;
    std::vector<int> wholeIndex;
    void createIndex(std::string source);
public:
    explicit LineDocument(std::string content, int maxLineLen = 0);
    std::optional<std::string_view> line(int n) override;
    int lineCount() override;
};
}

#endif //VIEWER_LINEDOCUMENT_H
