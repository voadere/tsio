/*
 * Copyright (C) 2018, Jacques Van Damme.
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 * 
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 * 
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef FORMATTER_H
#define FORMATTER_H

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string.h>
#include <string>

namespace tsioImplementation
{
extern bool initialized;
void initialize();

enum {
    zerofill = 1,
    signedNumber = zerofill << 1,
    plusIfPositive = signedNumber << 1,
    spaceIfPositive = plusIfPositive << 1,
    leftJustify = spaceIfPositive << 1,
    upcase = leftJustify << 1,
    alternative = upcase << 1
};

struct FormatState
{
    int specNum1 = 0;
    int specNum2 = 0;
    unsigned type = 0;
    bool specNum1Dynamic = false;
    bool specNum2Dynamic = false;
    bool specNum1Given = false;
    bool specNum2Given = false;
    bool active = false;
    char formatSpecifier = 0;

    mutable char buf[31]; // enough for 5 flags, 2 ints, a dot and a specifier.

    FormatState() = default;
    FormatState(const char*& format)
    {
        parse(format);
    }

    void parse(const char*& format);

    const char* unParse() const;

    void reset()
    {
        specNum1 = 0;
        specNum2 = 0;
        type = 0;
        specNum1Given = false;
        specNum2Given = false;
        specNum1Dynamic = false;
        specNum2Dynamic = false;
        active = false;
        formatSpecifier = 0;
    }
};

void outputString(std::string& dest, const char* text, size_t size, int minSize, int maxSize, unsigned type);

inline void outputString(std::string& dest, const char* text, int minSize, int maxSize, unsigned type)
{
    outputString(dest, text, strlen(text), minSize, maxSize, type);
}

void outputNumber(std::string& dest, long long pNumber, int base, int size, int precision, unsigned type);

void printfDetail(std::string& dest, const FormatState& state, char format, const std::string& value);
void printfDetail(std::string& dest, const FormatState& state, char format, const char* value);
void printfDetail(std::string& dest, const FormatState& state, char format, double value);

template <typename T>
inline void printfDetail(std::string& dest, const FormatState& state, char format, const T& value)
{
    static char niceChar[256] = {0};
    char charBuf[2];
    typename std::make_signed<T>::type sValue = value;
    typename std::make_unsigned<T>::type uValue = value;

    auto type = state.type;

    if ((type & zerofill) && state.specNum2Given) {
        type &= ~zerofill;
    }

    switch (format) {
        case 'b':
            outputNumber(dest, uValue, 2, state.specNum1, state.specNum2, type);
            break;

        case 'c':
            *charBuf = char(value);
            charBuf[1] = 0;
            outputString(dest,
                    charBuf,
                    state.specNum1,
                    state.specNum2Given ? (state.specNum2 > 0 ? state.specNum2 : 1)
                    : std::numeric_limits<int>::max(),
                    type);

            break;

        case 'C':
            if (*niceChar == 0) {
                memset(niceChar, '.', sizeof niceChar);

                for (int cnt = ' '; cnt <= '~'; ++cnt) {
                    niceChar[cnt] = char(cnt);
                }
            }

            *charBuf = niceChar[value & 0xff];
            charBuf[1] = 0;
            outputString(dest, charBuf, state.specNum1, state.specNum2, type);

            break;

        case 'd':
        case 'i':
        case 's':
            outputNumber(dest,
                    sValue,
                    10,
                    state.specNum1,
                    state.specNum2Given ? state.specNum2 : 1,
                    type | signedNumber);
            break;

        case 'o':
            outputNumber(dest, uValue, 8, state.specNum1, state.specNum2Given ? state.specNum2 : 1, type);

            break;

        case 'u':
            outputNumber(
                    dest, uValue, 10, state.specNum1, state.specNum2Given ? state.specNum2 : 1, type);

            break;

        case 'x':
            outputNumber(
                    dest, uValue, 16, state.specNum1, state.specNum2Given ? state.specNum2 : 1, type);

            break;

        case 'X':
            outputNumber(dest,
                    uValue,
                    16,
                    state.specNum1,
                    state.specNum2Given ? state.specNum2 : 1,
                    type | upcase);

            break;

        case 'n':
            std::cerr << "Did you forget to specify the parameter for '%n' by pointer?" << std::endl;

        default:
            std::cerr << "Invalid format '" << format << "' for integeral value" << std::endl;
    }
}

template <typename T>
inline void printfDetail(std::string& dest, const FormatState& state, char format, T* value)
{
    uintptr_t pValue = uintptr_t(value);

    switch (format) {
        case 'p':
            outputNumber(dest, pValue, 16, state.specNum1, state.specNum2, state.type | alternative);

            break;

        case 'n':
            using baseType = typename std::remove_pointer<T>::type;

            *value = baseType(dest.size());

            break;

        default:
            printfDetail(dest, state, format, static_cast<uintptr_t>(pValue));
    }
}

template <typename T>
inline void printfDetail(std::string& dest, const FormatState& state, char format, const T* value)
{
    uintptr_t pValue = uintptr_t(value);

    switch (format) {
        case 'p':
            outputNumber(dest, pValue, 16, state.specNum1, state.specNum2, state.type | alternative);

            break;

        default:
            printfDetail(dest, state, format, static_cast<uintptr_t>(pValue));
    }
}

inline void printfDetail(std::string& dest, const FormatState& state, char format, float value)
{
    printfDetail(dest, state, format, double(value));
}

inline void printfDetail(std::string& dest, const FormatState& state, char format, bool value)
{
    if (format == 's') {
        const char* pt = value ? "true" : "false";

        printfDetail(dest, state, format, pt);
    } else {
        printfDetail(dest, state, format, static_cast<long long>(value));
    }
}

template <typename T, typename enable = void>
struct ToSpec
{
    int operator()(const T&)
    {
        std::cerr << "invalid conversion" << std::endl;
        return 0;
    }
};

template <typename T>
struct ToSpec<T, typename std::enable_if<std::is_integral<T>::value>::type>
{
    int operator()(const T& value)
    {
        return int(value);
    }
};

template <typename T>
void printfOne(std::string& dest, const char*& format, FormatState& state, const T& value)
{
    ToSpec<T> toSpec;

    if (state.active) {
        if (state.specNum2Dynamic) {
            state.specNum2 = toSpec(value);
            state.specNum2Dynamic = false;
            return;
        }
    } else {
        const char* pt0 = format;

        while (*format != 0) {
            if (*format == '%') {
                if (format != pt0) {
                    dest.append(pt0, format - pt0);
                }

                format++;
                pt0 = format;

                if (*format != '%') {
                    break;
                }
            }

            format++;
        }

        if (*format == 0) {
            std::cerr << "Extraneous parameter or missing format specifier." << std::endl;
            return;
        }

        state.parse(format);

        if (state.active) {
            if (state.specNum1Dynamic) {
                state.specNum1 = toSpec(value);
                if (state.specNum1 < 0) {
                    state.specNum1 = -state.specNum1;
                    state.type |= leftJustify;
                }

                state.specNum1Dynamic = false;
                return;
            }

            if (state.specNum2Dynamic) {
                state.specNum2 = toSpec(value);
                state.specNum2Dynamic = false;
                return;
            }
        }
    }

    printfDetail(dest, state, state.formatSpecifier, value);
    state.reset();
}

#if __cplusplus < 201703L
template <typename T, typename... Ts>
inline void unpack(std::string& dest, const char*& format, FormatState& state, const T& t, const Ts&... ts)
{
    printfOne(dest, format, state, t);
    unpack(dest, format, state, ts...);
}

inline void unpack(std::string& dest, const char*& format, FormatState& state)
{
    // nop
}
#endif

template <typename... Ts>
void addsprintf(std::string& dest, const char* format, const Ts&... ts)
{
    FormatState state;

#if __cplusplus >= 201703L
    (printfOne(dest, format, state, ts), ...);
#else
    unpack(dest, format, state, ts...);
#endif

    while (*format != 0) {
        char c = *(format++);

        if (c == '%') {
            if (*format != '%') {
                std::cerr << "Missing parameter or extraneous format specifier." << std::endl;
            }

            format++;
        }

        dest.push_back(c);
    }
}
};

namespace tsio
{
class fmt
{
public:
    fmt(const char* format = nullptr)
    {
        initialize(format);
    }

    fmt(const std::string& format)
    {
        initialize(format.c_str());
    }

    std::ostream& operator()(std::ostream& out) const;

private:
    void initialize(const char* format);

    tsioImplementation::FormatState state;
};

inline std::ostream& operator<<(std::ostream& out, const fmt& f)
{
    return f(out);
}

template <typename... Ts>
int sprintf(std::string& dest, const char* format, const Ts&... ts)
{
    dest.clear();

    tsioImplementation::addsprintf(dest, format, ts...);

    return int(dest.size());
}

template <typename... Ts>
int addsprintf(std::string& dest, const char* format, const Ts&... ts)
{
    auto startSize = dest.size();

    tsioImplementation::addsprintf(dest, format, ts...);

    return int(dest.size() - startSize);
}

template <typename... Ts>
int fprintf(std::ostream& os, const char* format, const Ts&... ts)
{
    std::string tmp;

    tsioImplementation::addsprintf(tmp, format, ts...);
    os << tmp;

    return int(tmp.size());
}

template <typename... Ts>
std::string fstring(const char* format, const Ts&... ts)
{
    std::string tmp;

    tsioImplementation::addsprintf(tmp, format, ts...);

    return tmp;
}

};

#endif
