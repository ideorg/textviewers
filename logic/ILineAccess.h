//
// Created by andrzej on 8/23/23.
//

#ifndef VIEWER_ILINEACCESS_H
#define VIEWER_ILINEACCESS_H

#include <string_view>
#include <optional>
#include "IBaseAccess.h"

namespace vl {

class ILineAccess : public virtual IBaseAccess {
public:
    virtual std::optional<std::string_view> lineByIndex(int n) = 0;
    virtual int lineCount() = 0;
    virtual bool linesAreEmpty() = 0;
    virtual bool isFirstInFile(int n) = 0;
    virtual bool isLastInFile(int n) = 0;
};
}

#endif //VIEWER_ILINEACCESS_H
