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

struct SmallLinePoints {
    int64_t offset = 0;
    int len = 0;
};

class IByteAccess : public virtual Interface {
public:
    virtual int64_t firstByte() = 0;
    virtual int64_t byteCount() = 0;
    virtual bool fileIsEmpty() = 0;
    virtual std::optional<LinePoints> firstLine() = 0;
    virtual std::optional<LinePoints> lastLine() = 0;
    virtual LinePoints lineEnclosing(int64_t position) = 0;
    virtual std::string_view line(const LinePoints &linePoints) = 0;
    virtual std::string_view line(const SmallLinePoints &linePoints) = 0;
    virtual std::optional<LinePoints> lineBefore(const LinePoints &linePoints) = 0;
    virtual std::optional<LinePoints> lineAfter(const LinePoints &linePoints) = 0;
    virtual bool isFirstInFile(const LinePoints &linePoints) = 0;
    virtual bool isLastInFile(const LinePoints &linePoints) = 0;
    virtual int64_t pointerToOffset(const char *s) = 0;
    virtual const char *ofsetToPointer(int64_t pos) = 0;
};
}

#endif //VIEWER_IBYTEACCESS_H
