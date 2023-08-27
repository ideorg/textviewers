//
// Created by andrzej on 8/26/23.
//

#ifndef VIEWERS_CHANGEABLEDOCUMENT_H
#define VIEWERS_CHANGEABLEDOCUMENT_H

#include <vector>
#include <string>
#include <string_view>
#include "ILineAccess.h"

namespace vl {
class ChangeableDocument : public virtual ILineAccess {
    std::vector<std::string> stringList;
    void createStringList(std::string_view source);
public:
    explicit ChangeableDocument(std::string_view content, int maxLineLen = 0);
    std::optional<std::string_view> lineByIndex(int n) override;
    int lineCount() override;
    bool linesAreEmpty() override;
    bool isFirstInFile(int n) override;
    bool isLastInFile(int n) override;
};
}

#endif //VIEWERS_CHANGEABLEDOCUMENT_H
