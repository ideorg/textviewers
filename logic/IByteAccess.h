//
// Created by andrzej on 8/23/23.
//

#ifndef VIEWER_IBYTEACCESS_H
#define VIEWER_IBYTEACCESS_H

#include <string_view>
#include <optional>

class LinePoints {
private:
    int64_t offset = 0;
    int64_t len = 0;
    int64_t fullLen = 0;
};

class IByteAccess {
public:
    virtual std::optional<std::string_view> line(const LinePoints& linePoints) = 0;
    virtual std::optional<LinePoints> firstLine() = 0;
    virtual std::optional<LinePoints> lastLine() = 0;
    virtual std::optional<LinePoints> lineEnclosing(int64_t position) = 0;
    virtual std::optional<LinePoints> lineBefore(const LinePoints& linePoints) = 0;
    virtual std::optional<LinePoints> lineAfter(const LinePoints& linePoints) = 0;
};


#endif //VIEWER_IBYTEACCESS_H
