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
    IByteAccess *idoc = &byteDocument;
    int source_size = (int) source.size();
    if (source_size != source.size())
        throw runtime_error("file too large");
    auto opt = idoc->firstLine();
    if (!opt)
        return;
    while (opt) {
        auto lp = opt.value();
        string str(source.data()+lp.offset, lp.len);
        stringList.push_back(str);
        opt = idoc->lineAfter(lp);
    }
}

std::optional<std::string_view> ChangeableDocument::lineByIndex(int n) {
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

std::string_view ChangeableDocument::getBytes(FilePosition from, FilePosition to) {
    if (from.lineNumber == to.lineNumber) {
        if (to.offset <= from.offset)
            return {};
        else
            getStr = stringList[from.lineNumber].substr(from.offset, to.offset - from.offset);
    } else {
        getStr = stringList[from.lineNumber].substr(from.offset);
        for (size_t i = from.lineNumber; i < to.lineNumber; i++)
            getStr += "\n" + stringList[i];
        getStr += "\n" + stringList[to.lineNumber].substr(0, to.offset);
    }
    return {getStr};
}
