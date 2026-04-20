#define _GNU_SOURCE 1
#include <string.h>

#include "Searcher.h"

#include <algorithm>
#include <vector>

#include <unicode/utf8.h>
#include <unicode/uchar.h>

#include "IByteAccess.h"
#include "Wrap.h"

namespace vl {

Searcher::Searcher(IByteAccess *doc, int64_t chunkSize)
    : m_doc(doc), m_chunkSize(chunkSize) {}

// Byte-length-preserving lowercase fold: codepoints whose u_tolower has a
// different UTF-8 encoding length are copied unchanged.  This keeps the
// offset mapping from the folded buffer to the source buffer 1:1, so a
// memmem hit in the folded buffer translates back to an original file
// offset with a simple pointer subtraction.
void Searcher::foldByteRange(const char *src, size_t len, char *dst) {
    auto s = reinterpret_cast<const uint8_t *>(src);
    auto d = reinterpret_cast<uint8_t *>(dst);
    int32_t i = 0;
    int32_t n = (int32_t) len;
    while (i < n) {
        int32_t before = i;
        UChar32 c;
        U8_NEXT(s, i, n, c);
        int32_t origLen = i - before;
        if (c < 0) {
            for (int32_t k = before; k < i; k++) d[k] = s[k];
            continue;
        }
        UChar32 lc = u_tolower(c);
        if (U8_LENGTH(lc) != origLen) {
            for (int32_t k = before; k < i; k++) d[k] = s[k];
            continue;
        }
        int32_t writeAt = before;
        UBool isError = false;
        U8_APPEND(d, writeAt, n, lc, isError);
    }
}

std::string Searcher::foldPattern(std::string_view pattern) {
    std::string out(pattern.size(), '\0');
    foldByteRange(pattern.data(), pattern.size(), out.data());
    return out;
}

// Treat file start/end as a word boundary; otherwise require the adjacent
// codepoint to be non-word-class (codeClass != 1).
bool Searcher::isWordBoundary(int64_t matchStart, int64_t matchLen) {
    int64_t first = m_doc->firstByte();
    int64_t total = m_doc->byteCount();

    if (matchStart > first) {
        int64_t lookback = std::min<int64_t>(4, matchStart - first);
        const char *base = m_doc->ofsetToPointer(matchStart - lookback);
        auto u8 = reinterpret_cast<const uint8_t *>(base);
        int32_t i = (int32_t) lookback;
        U8_BACK_1(u8, 0, i);
        int32_t decodeAt = i;
        UChar32 c;
        U8_NEXT(u8, decodeAt, (int32_t) lookback, c);
        if (c < 0) c = 0xFFFD;
        if (Wrap::codeClass((char32_t) c) == 1) return false;
    }

    int64_t afterOffset = matchStart + matchLen;
    if (afterOffset < total) {
        int64_t lookforward = std::min<int64_t>(4, total - afterOffset);
        const char *base = m_doc->ofsetToPointer(afterOffset);
        auto u8 = reinterpret_cast<const uint8_t *>(base);
        int32_t i = 0;
        UChar32 c;
        U8_NEXT(u8, i, (int32_t) lookforward, c);
        if (c < 0) c = 0xFFFD;
        if (Wrap::codeClass((char32_t) c) == 1) return false;
    }

    return true;
}

std::optional<int64_t> Searcher::findNext(std::string_view pattern,
                                          int64_t startOffset,
                                          const ProgressFn &onProgress,
                                          const SearchOptions &options) {
    if (pattern.empty()) return std::nullopt;
    int64_t first = m_doc->firstByte();
    int64_t total = m_doc->byteCount();
    int64_t pLen = (int64_t) pattern.size();
    int64_t pos = std::max(startOffset, first);
    if (pos + pLen > total) return std::nullopt;
    int64_t rangeTotal = total - pos;

    std::string foldedPattern;
    std::vector<char> buf;
    const char *patternData = pattern.data();
    if (options.caseInsensitive) {
        foldedPattern = foldPattern(pattern);
        patternData = foldedPattern.data();
        buf.resize((size_t) (m_chunkSize + pLen - 1));
    }

    while (pos + pLen <= total) {
        int64_t chunkEnd = std::min(total, pos + m_chunkSize + pLen - 1);
        size_t chunkLen = (size_t) (chunkEnd - pos);
        const char *origBase = m_doc->ofsetToPointer(pos);
        const char *searchBase;
        if (options.caseInsensitive) {
            foldByteRange(origBase, chunkLen, buf.data());
            searchBase = buf.data();
        } else {
            searchBase = origBase;
        }

        const char *p = searchBase;
        size_t remaining = chunkLen;
        while (remaining >= (size_t) pLen) {
            void *hit = memmem(p, remaining, patternData, pattern.size());
            if (!hit) break;
            const char *hitP = (const char *) hit;
            int64_t hitOffset = pos + (hitP - searchBase);
            if (!options.wholeWord || isWordBoundary(hitOffset, pLen)) {
                return hitOffset;
            }
            p = hitP + 1;
            remaining = (size_t) ((searchBase + chunkLen) - p);
        }

        if (chunkEnd == total) break;
        pos += m_chunkSize;
        if (onProgress) {
            int64_t done = std::min(rangeTotal, pos - startOffset);
            if (!onProgress(done, rangeTotal)) return std::nullopt;
        }
    }
    return std::nullopt;
}

std::optional<int64_t> Searcher::findPrev(std::string_view pattern,
                                          int64_t endOffset,
                                          const ProgressFn &onProgress,
                                          const SearchOptions &options) {
    if (pattern.empty()) return std::nullopt;
    int64_t first = m_doc->firstByte();
    int64_t total = m_doc->byteCount();
    int64_t pLen = (int64_t) pattern.size();
    int64_t end = std::min(endOffset, total);
    if (end - first < pLen) return std::nullopt;
    int64_t rangeTotal = end - first;

    std::string foldedPattern;
    std::vector<char> buf;
    const char *patternData = pattern.data();
    if (options.caseInsensitive) {
        foldedPattern = foldPattern(pattern);
        patternData = foldedPattern.data();
        buf.resize((size_t) (m_chunkSize + pLen - 1));
    }

    while (end - first >= pLen) {
        int64_t begin = std::max(first, end - m_chunkSize - (pLen - 1));
        size_t chunkLen = (size_t) (end - begin);
        const char *origBase = m_doc->ofsetToPointer(begin);
        const char *searchBase;
        if (options.caseInsensitive) {
            foldByteRange(origBase, chunkLen, buf.data());
            searchBase = buf.data();
        } else {
            searchBase = origBase;
        }

        int64_t bestOffset = -1;
        const char *p = searchBase;
        size_t remaining = chunkLen;
        while (remaining >= (size_t) pLen) {
            void *hit = memmem(p, remaining, patternData, pattern.size());
            if (!hit) break;
            const char *hitP = (const char *) hit;
            int64_t hitInChunk = hitP - searchBase;
            int64_t hitOffset = begin + hitInChunk;
            if (hitOffset + pLen > end) break;
            if (!options.wholeWord || isWordBoundary(hitOffset, pLen)) {
                bestOffset = hitOffset;
            }
            p = hitP + 1;
            remaining = (size_t) ((searchBase + chunkLen) - p);
        }
        if (bestOffset >= 0) return bestOffset;

        if (begin == first) break;
        end -= m_chunkSize;
        if (onProgress) {
            int64_t done = std::min(rangeTotal, endOffset - end);
            if (!onProgress(done, rangeTotal)) return std::nullopt;
        }
    }
    return std::nullopt;
}

}