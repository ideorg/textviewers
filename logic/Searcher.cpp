#define _GNU_SOURCE 1
#include <string.h>

#include "Searcher.h"

#include <algorithm>

#include "IByteAccess.h"

namespace vl {

Searcher::Searcher(IByteAccess *doc, int64_t chunkSize)
    : m_doc(doc), m_chunkSize(chunkSize) {}

std::optional<int64_t> Searcher::findNext(std::string_view pattern,
                                          int64_t startOffset,
                                          const ProgressFn &onProgress) {
    if (pattern.empty()) return std::nullopt;
    int64_t first = m_doc->firstByte();
    int64_t total = m_doc->byteCount();
    int64_t pLen = (int64_t) pattern.size();
    int64_t pos = std::max(startOffset, first);
    if (pos + pLen > total) return std::nullopt;
    int64_t rangeTotal = total - pos;

    while (pos + pLen <= total) {
        int64_t chunkEnd = std::min(total, pos + m_chunkSize + pLen - 1);
        size_t chunkLen = (size_t) (chunkEnd - pos);
        const char *base = m_doc->ofsetToPointer(pos);
        void *hit = memmem(base, chunkLen, pattern.data(), pattern.size());
        if (hit) {
            return m_doc->pointerToOffset((const char *) hit);
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
                                          const ProgressFn &onProgress) {
    if (pattern.empty()) return std::nullopt;
    int64_t first = m_doc->firstByte();
    int64_t total = m_doc->byteCount();
    int64_t pLen = (int64_t) pattern.size();
    int64_t end = std::min(endOffset, total);
    if (end - first < pLen) return std::nullopt;
    int64_t rangeTotal = end - first;

    while (end - first >= pLen) {
        int64_t begin = std::max(first, end - m_chunkSize - (pLen - 1));
        size_t chunkLen = (size_t) (end - begin);
        const char *base = m_doc->ofsetToPointer(begin);

        int64_t bestOffset = -1;
        const char *p = base;
        size_t remaining = chunkLen;
        while (remaining >= (size_t) pLen) {
            void *hit = memmem(p, remaining, pattern.data(), pattern.size());
            if (!hit) break;
            const char *hitP = (const char *) hit;
            int64_t hitOffset = m_doc->pointerToOffset(hitP);
            if (hitOffset + pLen > end) break;
            bestOffset = hitOffset;
            p = hitP + 1;
            remaining = (size_t) ((base + chunkLen) - p);
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