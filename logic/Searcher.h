#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace vl {

class IByteAccess;

struct SearchOptions {
    // Case-insensitive matching via Unicode simple lowercase folding.
    // Only codepoints whose lowercase has the same UTF-8 byte length are
    // folded; others are left unchanged (so e.g. ASCII A-Z, polish ĄĆĘŁŃÓŚŹŻ,
    // Cyrillic, Greek fold as expected, while İ/ß stay case-sensitive).
    bool caseInsensitive = false;
    // Only accept matches that are not adjacent to a word-class codepoint.
    bool wholeWord = false;
};

class Searcher {
public:
    // Return false to cancel the search.
    using ProgressFn = std::function<bool(int64_t bytesDone, int64_t bytesTotal)>;

    explicit Searcher(IByteAccess *doc, int64_t chunkSize = 16 * 1024 * 1024);

    // Returns offset of the earliest match whose start is >= startOffset
    // and whose end is <= limit (-1: end of file).
    // Returns nullopt if no match, pattern is empty, or search is cancelled.
    std::optional<int64_t> findNext(std::string_view pattern,
                                    int64_t startOffset,
                                    const ProgressFn &onProgress,
                                    const SearchOptions &options = {},
                                    int64_t limit = -1);

    // Returns offset of the latest match whose end is <= endOffset
    // and whose start is >= limit (-1: start of file).
    // Returns nullopt if no match, pattern is empty, or search is cancelled.
    std::optional<int64_t> findPrev(std::string_view pattern,
                                    int64_t endOffset,
                                    const ProgressFn &onProgress,
                                    const SearchOptions &options = {},
                                    int64_t limit = -1);

    // Non-overlapping matches starting in [begin, end); a match may extend
    // past end. Meant for small ranges such as the visible part of a file.
    std::vector<int64_t> findAll(std::string_view pattern,
                                 int64_t begin, int64_t end,
                                 const SearchOptions &options = {});

private:
    IByteAccess *m_doc;
    int64_t m_chunkSize;
    static void foldByteRange(const char *src, size_t len, char *dst);
    static std::string foldPattern(std::string_view pattern);
    bool isWordBoundary(int64_t matchStart, int64_t matchLen);
};

}