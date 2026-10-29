#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include "Searcher.h"

namespace vl {

class IByteAccess;

struct RegexMatch {
    int64_t offset = 0;
    int64_t length = 0;
};

// Regular expression search over the bytes of a file (PCRE2, UTF-8, JIT).
// The file is matched in chunks that overlap by maxMatchLength bytes, so a
// match longer than that may be missed or shortened. ^ and $ match at line
// starts and ends; empty matches are never reported.
class RegexSearcher {
public:
    using ProgressFn = Searcher::ProgressFn;
    static constexpr int64_t maxMatchLength = 64 * 1024;

    // caseInsensitive and wholeWord of options apply as in Searcher.
    RegexSearcher(IByteAccess *doc, std::string_view pattern,
                  const SearchOptions &options = {},
                  int64_t chunkSize = 16 * 1024 * 1024);
    ~RegexSearcher();
    RegexSearcher(const RegexSearcher &) = delete;
    RegexSearcher &operator=(const RegexSearcher &) = delete;

    bool isValid() const;
    // The compile error of an invalid pattern, and its offset in the pattern.
    std::string errorString() const { return m_error; }
    int64_t errorOffset() const { return m_errorOffset; }

    // The earliest match starting at or after startOffset and ending at or
    // before limit (-1: end of file).
    std::optional<RegexMatch> findNext(int64_t startOffset,
                                       const ProgressFn &onProgress,
                                       int64_t limit = -1);

    // The last match ending at or before endOffset and starting at or after
    // limit (-1: start of file).
    std::optional<RegexMatch> findPrev(int64_t endOffset,
                                       const ProgressFn &onProgress,
                                       int64_t limit = -1);

    // Non-overlapping matches starting in [begin, end); a match may extend
    // past end. Meant for small ranges such as the visible part of a file.
    std::vector<RegexMatch> findAll(int64_t begin, int64_t end);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    IByteAccess *m_doc;
    int64_t m_chunkSize;
    std::string m_error;
    int64_t m_errorOffset = -1;
    // Calls onMatch for the non-overlapping matches starting in [begin, end),
    // in order, until it returns false.
    template<class F>
    bool scan(int64_t begin, int64_t end, F &&onMatch);
};

}
