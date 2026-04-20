#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <unicode/utf8.h>
#include <unicode/utf16.h>

struct UTF {
    static constexpr uint8_t MAXCHARLEN = 6;
    static constexpr char32_t MaxCP = 0x10ffff;

    static uint8_t one8len(char c) {
        auto b = (uint8_t)c;
        if (U8_IS_SINGLE(b)) return 1;
        uint8_t n = U8_COUNT_TRAIL_BYTES(b);
        return n ? n + 1 : 1;
    }

    static char32_t one8to32(const char *s, const char *eos, const char **end) {
        auto u8 = reinterpret_cast<const uint8_t*>(s);
        int32_t length = (int32_t)(eos - s);
        int32_t i = 0;
        UChar32 c;
        U8_NEXT(u8, i, length, c);
        if (c < 0) c = 0xFFFD;
        *end = s + i;
        return (char32_t)c;
    }

    static uint8_t one32to8(char32_t d, char *buf) {
        if (d > MaxCP || U_IS_SURROGATE(d)) d = 0xFFFD;
        auto u8 = reinterpret_cast<uint8_t*>(buf);
        int32_t i = 0;
        UBool isError = false;
        U8_APPEND(u8, i, 4, (UChar32)d, isError);
        return (uint8_t)i;
    }

    static std::u32string u8to32(std::string_view str) {
        std::u32string result;
        result.reserve(str.size());
        auto u8 = reinterpret_cast<const uint8_t*>(str.data());
        int32_t length = (int32_t)str.size();
        int32_t i = 0;
        while (i < length) {
            UChar32 c;
            U8_NEXT(u8, i, length, c);
            if (c < 0) c = 0xFFFD;
            result.push_back((char32_t)c);
        }
        return result;
    }

    static int64_t numCodesBetween(const char *s, const char *s1) {
        auto u8 = reinterpret_cast<const uint8_t*>(s);
        int32_t length = (int32_t)(s1 - s);
        int32_t i = 0;
        int64_t n = 0;
        while (i < length) {
            U8_FWD_1(u8, i, length);
            n++;
        }
        return n;
    }

    static const char *forwardNcodes(const char *s, int64_t N, const char *send, int64_t &actual) {
        auto u8 = reinterpret_cast<const uint8_t*>(s);
        int32_t length = (int32_t)(send - s);
        int32_t i = 0;
        actual = 0;
        for (int64_t n = 0; n < N && i < length; n++) {
            U8_FWD_1(u8, i, length);
            actual++;
        }
        return s + i;
    }

    static const char *backwardNcodes(const char *s, int64_t N, const char *sstart, int64_t &actual) {
        auto u8 = reinterpret_cast<const uint8_t*>(sstart);
        int32_t i = (int32_t)(s - sstart);
        actual = 0;
        for (int64_t n = 0; n < N && i > 0; n++) {
            U8_BACK_1(u8, 0, i);
            actual++;
        }
        return sstart + i;
    }

    static const char *findNextUtf8OrTheSame(const char *s, const char *ss, const char *eos) {
        auto u8 = reinterpret_cast<const uint8_t*>(ss);
        int32_t length = (int32_t)(eos - ss);
        int32_t i = (int32_t)(s - ss);
        int32_t i_orig = i;
        U8_SET_CP_START(u8, 0, i);
        if (i == i_orig) return s;
        U8_FWD_1(u8, i, length);
        return ss + i;
    }
};