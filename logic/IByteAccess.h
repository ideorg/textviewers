//
// Created by andrzej on 8/23/23.
//

#ifndef VIEWER_IBYTEACCESS_H
#define VIEWER_IBYTEACCESS_H

#include <string_view>
#include <optional>
#include "Interface.h"

namespace vl {

struct LinePoints {
    int64_t offset = 0;
    int64_t len = 0;
    int64_t fullLen = 0;
};

class IByteAccess : public Interface {
public:
    virtual int64_t byteCount() = 0;
    virtual std::optional<LinePoints> firstLine() = 0;
    virtual std::optional<LinePoints> lastLine() = 0;
    virtual LinePoints lineEnclosing(int64_t position) = 0;
    virtual std::string_view line(const LinePoints &linePoints) = 0;
    virtual std::optional<LinePoints> lineBefore(const LinePoints &linePoints) = 0;
    virtual std::optional<LinePoints> lineAfter(const LinePoints &linePoints) = 0;
    virtual bool isFirstInFile(const LinePoints &linePoints) = 0;
    virtual bool isLastInFile(const LinePoints &linePoints) = 0;
};
}

#endif //VIEWER_IBYTEACCESS_H
