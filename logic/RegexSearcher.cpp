#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "RegexSearcher.h"
#include "IByteAccess.h"

#include <algorithm>

namespace vl {

struct RegexSearcher::Impl {
    pcre2_code *code = nullptr;
    pcre2_match_data *matchData = nullptr;
    pcre2_match_context *matchContext = nullptr;
    pcre2_jit_stack *jitStack = nullptr;
    // Bytes before a match start that lookbehinds, \b and ^ may look at.
    int64_t contextBytes = 4;

    ~Impl() {
        pcre2_match_data_free(matchData);
        pcre2_match_context_free(matchContext);
        pcre2_jit_stack_free(jitStack);
        pcre2_code_free(code);
    }
};

RegexSearcher::RegexSearcher(IByteAccess *doc, std::string_view pattern,
                             const SearchOptions &options, int64_t chunkSize)
    : m_impl(std::make_unique<Impl>()), m_doc(doc), m_chunkSize(chunkSize) {
    if (pattern.empty()) {
        m_error = "empty pattern";
        return;
    }
    static constexpr std::string_view wordPrefix = "(?<!\\w)(?:";
    std::string expression(pattern);
    if (options.wholeWord)
        expression = std::string(wordPrefix) + expression + ")(?!\\w)";
    // MATCH_INVALID_UTF: files may hold invalid UTF-8, and the subject is not
    // validated on every call (which would cost O(chunk) per match).
    uint32_t flags = PCRE2_UTF | PCRE2_UCP | PCRE2_MULTILINE | PCRE2_MATCH_INVALID_UTF;
    if (options.caseInsensitive) flags |= PCRE2_CASELESS;
    int errorCode = 0;
    PCRE2_SIZE errorOffset = 0;
    m_impl->code = pcre2_compile(reinterpret_cast<PCRE2_SPTR>(expression.data()),
                                 expression.size(), flags, &errorCode, &errorOffset, nullptr);
    if (!m_impl->code) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errorCode, buffer, sizeof buffer);
        m_error = reinterpret_cast<const char *>(buffer);
        int64_t offset = (int64_t) errorOffset;
        if (options.wholeWord) offset -= (int64_t) wordPrefix.size();
        m_errorOffset = std::clamp<int64_t>(offset, 0, (int64_t) pattern.size());
        return;
    }
    // Without JIT (e.g. unsupported CPU) the interpreter is used.
    if (pcre2_jit_compile(m_impl->code, PCRE2_JIT_COMPLETE) == 0) {
        m_impl->jitStack = pcre2_jit_stack_create(32 * 1024, 1024 * 1024, nullptr);
        m_impl->matchContext = pcre2_match_context_create(nullptr);
        pcre2_jit_stack_assign(m_impl->matchContext, nullptr, m_impl->jitStack);
    }
    m_impl->matchData = pcre2_match_data_create_from_pattern(m_impl->code, nullptr);
    uint32_t lookbehind = 0;
    pcre2_pattern_info(m_impl->code, PCRE2_INFO_MAXLOOKBEHIND, &lookbehind);
    m_impl->contextBytes = ((int64_t) lookbehind + 1) * 4;
}

RegexSearcher::~RegexSearcher() = default;

bool RegexSearcher::isValid() const {
    return m_impl->code != nullptr;
}

template<class F>
bool RegexSearcher::scan(int64_t begin, int64_t end, F &&onMatch) {
    int64_t first = m_doc->firstByte();
    int64_t fileEnd = m_doc->byteCount();
    begin = std::max(begin, first);
    end = std::min(end, fileEnd);
    if (begin >= end) return true;
    int64_t subjectStart = std::max(first, begin - m_impl->contextBytes);
    int64_t subjectEnd = std::min(fileEnd, end + maxMatchLength);
    auto subject = reinterpret_cast<PCRE2_SPTR>(m_doc->ofsetToPointer(subjectStart));
    auto length = (PCRE2_SIZE) (subjectEnd - subjectStart);
    uint32_t flags = PCRE2_NOTEMPTY;
    // The subject is cut out of the file: its ends are not line ends.
    if (subjectStart > first) flags |= PCRE2_NOTBOL;
    if (subjectEnd < fileEnd) flags |= PCRE2_NOTEOL;
    auto offset = (PCRE2_SIZE) (begin - subjectStart);
    while (offset < length) {
        int rc = pcre2_match(m_impl->code, subject, length, offset, flags,
                             m_impl->matchData, m_impl->matchContext);
        if (rc < 0) break;
        const PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(m_impl->matchData);
        int64_t matchStart = subjectStart + (int64_t) ovector[0];
        if (matchStart >= end) break;
        if (!onMatch(RegexMatch{matchStart, (int64_t) (ovector[1] - ovector[0])}))
            return false;
        offset = ovector[1];
    }
    return true;
}

std::optional<RegexMatch> RegexSearcher::findNext(int64_t startOffset,
                                                  const ProgressFn &onProgress,
                                                  int64_t limit) {
    if (!isValid()) return std::nullopt;
    int64_t high = m_doc->byteCount();
    if (limit >= 0) high = std::min(high, limit);
    int64_t pos = std::max(startOffset, m_doc->firstByte());
    int64_t rangeTotal = high - pos;
    int64_t from = pos;
    while (pos < high) {
        int64_t chunkEnd = std::min(high, pos + m_chunkSize);
        std::optional<RegexMatch> found;
        scan(pos, chunkEnd, [&](const RegexMatch &m) {
            if (m.offset + m.length > high) return true;
            found = m;
            return false;
        });
        if (found) return found;
        pos = chunkEnd;
        if (onProgress && pos < high && !onProgress(pos - from, rangeTotal))
            return std::nullopt;
    }
    return std::nullopt;
}

std::optional<RegexMatch> RegexSearcher::findPrev(int64_t endOffset,
                                                  const ProgressFn &onProgress,
                                                  int64_t limit) {
    if (!isValid()) return std::nullopt;
    int64_t low = m_doc->firstByte();
    if (limit >= 0) low = std::max(low, limit);
    int64_t end = std::min(endOffset, m_doc->byteCount());
    int64_t rangeTotal = end - low;
    int64_t chunkEnd = end;
    while (chunkEnd > low) {
        int64_t chunkBegin = std::max(low, chunkEnd - m_chunkSize);
        std::optional<RegexMatch> last;
        scan(chunkBegin, chunkEnd, [&](const RegexMatch &m) {
            if (m.offset + m.length <= end) last = m;
            return true;
        });
        if (last) return last;
        chunkEnd = chunkBegin;
        if (onProgress && chunkEnd > low && !onProgress(end - chunkEnd, rangeTotal))
            return std::nullopt;
    }
    return std::nullopt;
}

std::vector<RegexMatch> RegexSearcher::findAll(int64_t begin, int64_t end) {
    std::vector<RegexMatch> result;
    if (!isValid()) return result;
    scan(begin, end, [&](const RegexMatch &m) {
        result.push_back(m);
        return true;
    });
    return result;
}

}
