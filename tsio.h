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

#include <array>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string.h>
#include <string>
#include <vector>

namespace tsioImplementation
{
enum {
    numericfill = 1,
    alfafill = numericfill << 1,
    signedNumber = numericfill << 1,
    plusIfPositive = signedNumber << 1,
    spaceIfPositive = plusIfPositive << 1,
    leftJustify = spaceIfPositive << 1,
    centerJustify = leftJustify << 1,
    upcase = leftJustify << 1,
    alternative = upcase << 1,
    nice = alternative << 1
};

struct FormatState
{
    unsigned width = 0;
    unsigned precision = 0;
    unsigned type = 0;
    unsigned position = 0;
    unsigned widthPosition = 0;
    unsigned precisionPosition = 0;
    bool widthDynamic = false;
    bool precisionDynamic = false;
    bool widthGiven = false;
    bool precisionGiven = false;
    bool active = false;
    char formatSpecifier = 0;
    char fillCharacter = ' ';

    mutable char buf[31]; // enough for 5 flags, 2 ints, a dot and a specifier.

    FormatState() = default;
    FormatState(const char*& format);

    void parse(const char*& format);

    const char* unParse() const;

    void reset()
    {
        width = 0;
        precision = 0;
        type = 0;
        position = 0;
        widthPosition = 0;
        precisionPosition = 0;
        widthGiven = false;
        precisionGiven = false;
        widthDynamic = false;
        precisionDynamic = false;
        active = false;
        formatSpecifier = 0;
        fillCharacter = ' ';
    }
};

struct Format
{
    Format(std::string& d, const char* f)
        : format(f), dest(d)
    {
    }

    struct StackElement
    {
        StackElement(const char* f, size_t c)
            : format(f), count(c)
        {
        }

        const char* format;
        size_t count;
    };

    void push(size_t count)
    {
        if (count != 0) {
            stack.emplace_back(format, count - 1);
        }
    }

    bool repeat()
    {
        if (!stack.empty()) {
            if (stack.back().count-- == 0) {
                stack.pop_back();
                return false;
            }

            format = stack.back().format;
            return true;
        }

        return false;
    }

    const char* format;
    FormatState state;
    std::string& dest;
    std::vector<StackElement> stack;
};

void outputString(std::string& dest, const char* text, size_t size, int minSize, int maxSize, unsigned type, char fillCharacter);

inline void outputString(std::string& dest, const char* text, int minSize, int maxSize, unsigned type, char fillCharacter)
{
    outputString(dest, text, strlen(text), minSize, maxSize, type, fillCharacter);
}

void outputNumber(std::string& dest, long long pNumber, int base, int size, int precision, unsigned type, char fillCharacter);

void printfDetail(std::string& dest, const FormatState& state, const std::string& value);
void printfDetail(std::string& dest, const FormatState& state, const char* value);
void printfDetail(std::string& dest, const FormatState& state, double value);
void printfDetail(std::string& dest, const FormatState& state, float value);
void printfDetail(std::string& dest, const FormatState& state, bool value);

void skipToFormat(Format& format);

template <typename T>
inline void printfDetail(std::string& dest, const FormatState& state, const T& value)
{
    typename std::make_signed<T>::type sValue = value;
    typename std::make_unsigned<T>::type uValue = value;
    char format = state.formatSpecifier;
    char fillCharacter = state.fillCharacter;

    auto type = state.type;

    if ((type & numericfill) && state.precisionGiven) {
        type &= ~numericfill;
        fillCharacter = ' ';
    }

    switch (format) {
        case 'b':
            outputNumber(dest, uValue, 2, state.width, state.precision, type, fillCharacter);
            break;

        case 'c': {
            char c = char(value);
            outputString(dest,
                    &c,
                    1,
                    state.width,
                    state.precisionGiven ? (state.precision > 0 ? state.precision : 1)
                    : std::numeric_limits<int>::max(),
                    type,
                    fillCharacter);
            }

            break;

        case 'C': {
            char c = value & 0xff;
            outputString(dest,
                    &c,
                    1,
                    state.width,
                    state.precisionGiven ? (state.precision > 0 ? state.precision : 1)
                    : std::numeric_limits<int>::max(),
                    type | nice,
                    fillCharacter);

                  }

            break;

        case 'd':
        case 'i':
            outputNumber(dest,
                    sValue,
                    10,
                    state.width,
                    state.precisionGiven ? state.precision : 1,
                    type | signedNumber,
                    fillCharacter);
            break;

        case 's':
            if (std::is_signed<T>::value) {
                outputNumber(dest,
                        sValue,
                        10,
                        state.width,
                        state.precisionGiven ? state.precision : 1,
                        type | signedNumber,
                        fillCharacter);
            } else {
                outputNumber(
                        dest, uValue, 10, state.width, state.precisionGiven ? state.precision : 1, type,
                        fillCharacter);
            }

            break;

        case 'o':
            outputNumber(dest, uValue, 8, state.width, state.precisionGiven ? state.precision : 1, type,
                    fillCharacter);

            break;

        case 'u':
            outputNumber(
                    dest, uValue, 10, state.width, state.precisionGiven ? state.precision : 1, type,
                    fillCharacter);

            break;

        case 'x':
            outputNumber(
                    dest, uValue, 16, state.width, state.precisionGiven ? state.precision : 1, type,
                    fillCharacter);

            break;

        case 'X':
            outputNumber(dest,
                    uValue,
                    16,
                    state.width,
                    state.precisionGiven ? state.precision : 1,
                    type | upcase,
                    fillCharacter);

            break;

        case 'n':
            std::cerr << "Did you forget to specify the parameter for '%n' by pointer?" << std::endl;

        default:
            std::cerr << "Invalid format '" << format << "' for integeral value" << std::endl;
    }
}

template <typename T>
inline void printfDetail(std::string& dest, const FormatState& state, T* value)
{
    char format = state.formatSpecifier;
    uintptr_t pValue = uintptr_t(value);

    switch (format) {
        case 'p':
            outputNumber(dest, pValue, 16, state.width, state.precision, state.type | alternative,
                    state.fillCharacter);

            break;

        case 'n':
            using baseType = typename std::remove_pointer<T>::type;

            *value = baseType(dest.size());

            break;

        default:
            printfDetail(dest, state, static_cast<uintptr_t>(pValue));
    }
}

template <typename T>
inline void printfDetail(std::string& dest, const FormatState& state, const T* value)
{
    char format = state.formatSpecifier;
    uintptr_t pValue = uintptr_t(value);

    switch (format) {
        case 'p':
            outputNumber(dest, pValue, 16, state.width, state.precision, state.type | alternative,
                state.fillCharacter);

            break;

        default:
            printfDetail(dest, state, static_cast<uintptr_t>(pValue));
    }
}

template <typename T>
inline void printfDetail(std::string& dest, const FormatState& state, const std::vector<T>& value)
{
    for (const auto& v : value) {
        printfDetail(dest, state, v);
    };
}

template <typename T, size_t N>
inline void printfDetail(std::string& dest, const FormatState& state, const std::array<T, N>& value)
{
    for (const auto& v : value) {
        printfDetail(dest, state, v);
    };
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

void skipAfter(const char*& format, char startChar, char endChar);

template <typename T>
void printfOne(Format& format, const T& value)
{
    ToSpec<T> toSpec;
    FormatState& state = format.state;
    std::string& dest = format.dest;

    if (state.formatSpecifier == 0) {
        std::cerr << "Extraneous parameter or missing format specifier." << std::endl;
        return;
    }

    if (state.active) {
        if (state.widthDynamic) {
            int spec = toSpec(value);

            if (spec < 0) {
                state.width = -spec;
                state.type |= leftJustify;
                if (state.type & numericfill) {
                    state.type &= ~numericfill;
                    state.fillCharacter = ' ';
                }
            } else {
                state.width = spec;
            }

            state.widthDynamic = false;
            return;
        }

        if (state.precisionDynamic) {
            int spec = toSpec(value);

            if (spec < 0) {
                state.precisionGiven = false;
            } else {
                state.precision = spec;
            }

            state.precisionDynamic = false;
            return;
        }
    }

    while (state.formatSpecifier == '{') {
        if (state.width == 0) {
            skipAfter(format.format, '{', '}');
        } else {
            format.push(state.width);
        }

        state.reset();
        skipToFormat(format);
        state.parse(format.format);

        if (state.formatSpecifier == '{' && state.active) {
            int spec = toSpec(value);

            if (spec < 0) {
                spec = 0;
            }

            state.width = spec;
            return;
        }
    }

    printfDetail(dest, state, value);
    state.reset();
    skipToFormat(format);
    state.parse(format.format);
}

#if __cplusplus < 201703L
inline void printfNth(std::string&, FormatState&, size_t)
{
    std::cerr << "Invalid position in format." << std::endl;
}

template <typename T, typename... Ts>
void printfNth(std::string& dest, FormatState& state, size_t index, const T& t, const Ts&... ts)
{
    if (index == 1) {
        printfDetail(dest, state, t);
    } else {
        printfNth(dest, state, index - 1, ts...);
    }
}
inline void readSpecNum(int& dest, FormatState&, size_t)
{
    std::cerr << "Invalid position in format." << std::endl;
    dest = 0;
}

template <typename T, typename... Ts>

void readSpecNum(int& dest, FormatState& state, size_t index, const T& t, const Ts&... ts)
{
    ToSpec<T> toSpec;

    if (index == 1) {
        dest = toSpec(t);
    } else {
        readSpecNum(dest, state, index - 1, ts...);
    }
}
#else
template <typename T, typename... Ts>
bool printfDispatch(std::string& dest, FormatState& state, size_t index, const T& t, const Ts&... ts)
{
    if (index == 1) {
        printfDetail(dest, state, t);
        return true;
    } else {
        return false;
    }
}

template <typename... Ts>
void printfNth(std::string& dest, FormatState& state, size_t index,  const Ts&... ts)
{
    auto i = state.position + 1;

    static_cast<void>(((i--, printfDispatch(dest, state, i, ts)) || ...));
}

template <typename T, typename... Ts>
bool readSpecNumDispatch(int& dest, FormatState& state, size_t index, const T& t, const Ts&... ts)
{
    ToSpec<T> toSpec;

    if (index == 1) {
        dest = toSpec(t);
        return true;
    } else {
        return false;
    }
}

template <typename... Ts>
void readSpecNum(int& dest, FormatState& state, size_t index, const Ts&... ts)
{
    auto i = index + 1;

    static_cast<void>(((i--, readSpecNumDispatch(dest, state, i, ts)) || ...));
}

#endif

template <typename... Ts>
void printfPositionalOne(Format& format, const Ts&... ts)
{
    std::string& dest = format.dest;
    FormatState& state = format.state;

    if (state.formatSpecifier == 0) {
        std::cerr << "Extraneous parameter or missing format specifier." << std::endl;
        return;
    }

    if (state.position == 0) {
        std::cerr << "Positional arguments can not be mixed with sequential arguments." << std::endl;
        return;
    }

    if (state.active) {
        if (state.widthDynamic) {
            int spec = 0;

            readSpecNum(spec, state, state.widthPosition, ts...);

            if (spec < 0) {
                state.width = -spec;
                state.type |= leftJustify;
            } else {
                state.width = spec;
            }
        }

        if (state.precisionDynamic) {
            int spec = 0;

            readSpecNum(spec, state, state.precisionPosition, ts...);

            if (spec < 0) {
                state.precisionGiven = false;
            } else {
                state.precision = spec;
            }
        }
    }

    printfNth(dest, state, state.position, ts...);

    skipToFormat(format);
    state.reset();
    state.parse(format.format);

}

#if __cplusplus < 201703L
template <typename T, typename... Ts>
inline void unpack(Format& format, const T& t, const Ts&... ts)
{
    printfOne(format, t);
    unpack(format, ts...);
}

inline void unpack(Format&)
{
    // nop
}
#endif

template <typename... Ts>
void addsprintf(std::string& dest, const char* f, const Ts&... ts)
{
    Format format(dest, f);
    FormatState& state = format.state;

    skipToFormat(format);
    state.parse(format.format);

    while (state.formatSpecifier == '{' && !state.active) {
        if (state.width == 0) {
            skipAfter(format.format, '{', '}');
        } else {
            format.push(state.width);
        }

        state.reset();
        skipToFormat(format);
        state.parse(format.format);
    }

    if (state.position != 0) {
        do {
            printfPositionalOne(format, ts...);
        } while (*format.format != 0);
    } else {

#if __cplusplus >= 201703L
        (printfOne(format, ts), ...);
#else
        unpack(format, ts...);
#endif

        while (state.formatSpecifier == '{') {
            if (state.width == 0) {
                skipAfter(format.format, '{', '}');
            } else {
                format.push(state.width);
            }

            state.reset();
            skipToFormat(format);
            state.parse(format.format);

            if (state.formatSpecifier == '{' && state.active) {
                break;
            }
        }

        if (state.formatSpecifier != 0) {
            std::cerr << "Missing parameter or extraneous format specifier." << std::endl;
        }
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

template <typename... Arguments>
int sprintf(std::string& dest, const char* format, const Arguments&... arguments)
{
    dest.clear();

    tsioImplementation::addsprintf(dest, format, arguments...);

    return int(dest.size());
}

template <typename... Arguments>
int addsprintf(std::string& dest, const char* format, const Arguments&... arguments)
{
    auto startSize = dest.size();

    tsioImplementation::addsprintf(dest, format, arguments...);

    return int(dest.size() - startSize);
}

template <typename... Arguments>
int fprintf(std::ostream& os, const char* format, const Arguments&... arguments)
{
    std::string tmp;

    tsioImplementation::addsprintf(tmp, format, arguments...);
    os << tmp;

    return int(tmp.size());
}

template <typename... Arguments>
int oprintf(const char* format, const Arguments&... arguments)
{
    std::string tmp;

    tsioImplementation::addsprintf(tmp, format, arguments...);
    std::cout << tmp;

    return int(tmp.size());
}

template <typename... Arguments>
int eprintf(const char* format, const Arguments&... arguments)
{
    std::string tmp;

    tsioImplementation::addsprintf(tmp, format, arguments...);
    std::cerr << tmp;

    return int(tmp.size());
}

template <typename... Arguments>
std::string fstring(const char* format, const Arguments&... arguments)
{
    std::string tmp;

    tsioImplementation::addsprintf(tmp, format, arguments...);

    return tmp;
}

};

#endif
