//
// Created by andrzej on 8/26/23.
//

#include <stdexcept>
#include <cassert>
#include "ChangeableDocument.h"
#include "ByteDocument.h"

using namespace std;
using namespace vl;
ChangeableDocument::ChangeableDocument(std::string_view content, int maxLineLen) {
    createStringList(content);
}

void ChangeableDocument::createStringList(std::string_view source) {
    ByteDocument byteDocument(source.data(), source.size());
    int source_size = (int) source.size();
    if (source_size != source.size())
        throw runtime_error("file too large");
    int position = byteDocument.BOMsize();
    while (position < source_size) {
        int eolPos = byteDocument.searchEndOfLine(position);
        int len = eolPos - position;
        string str(source.data()+position, len);
        stringList.push_back(str);
        position = byteDocument.skipLineBreakEx(eolPos, len);
    }
    assert(position == source_size);
}

std::optional<std::string_view> ChangeableDocument::line(int n) {
    if (n<0 || n>= lineCount())
        return nullopt;
    auto &str = stringList[n];
    string_view view(str.data(), str.size());
    return make_optional<string_view>(view);
}

int ChangeableDocument::lineCount() {
    return stringList.size();
}

bool ChangeableDocument::linesAreEmpty() {
    return lineCount()==0;
}

bool ChangeableDocument::isFirstInFile(int n) {
    return n = 0;
}

bool ChangeableDocument::isLastInFile(int n) {
    return n == lineCount() - 1;
}
