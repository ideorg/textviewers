//
// Created by andrzej on 8/23/23.
//

#ifndef VIEWER_ILINEACCESS_H
#define VIEWER_ILINEACCESS_H

#include <string_view>
#include <optional>

class ILineAccess {
public:
    virtual std::optional<std::string_view> line(int n) = 0;
    virtual int lineCount() = 0;
};


#endif //VIEWER_ILINEACCESS_H
