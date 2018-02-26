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

#include "tsio.h"
#include <time.h>

#if defined(_MSC_VER)
#define ALWAYS_INLINE __forceinline
#else
#define ALWAYS_INLINE __attribute__((always_inline))
#endif

ALWAYS_INLINE void copy(char* dest, const char* src, unsigned size)
{
    switch (size) {
        case 10:
            dest[9] = src[9];
            // Fallthru
        case 9:
            dest[8] = src[8];
            // Fallthru
        case 8:
            dest[7] = src[7];
            // Fallthru
        case 7:
            dest[6] = src[6];
            // Fallthru
        case 6:
            dest[5] = src[5];
            // Fallthru
        case 5:
            dest[4] = src[4];
            // Fallthru
        case 4:
            dest[3] = src[3];
            // Fallthru
        case 3:
            dest[2] = src[2];
            // Fallthru
        case 2:
            dest[1] = src[1];
            // Fallthru
        case 1:
            dest[0] = src[0];
            // Fallthru
        case 0:
            return;
        default:
            memcpy(dest, src, size);
    }
}

void tsioImplementation::outputString(std::string& dest,
                                      const char* text,
                                      size_t s,
                                      int minSize,
                                      int maxSize,
                                      unsigned type,
                                      char fillCharacter)
{
    if (text == nullptr) {
        return;
    }

    std::string tmp;
    int size = int(s);

    if (type & nice) {
        if (type & alternative) {
            for (int i = 0; i < size; ++i) {
                char c = text[i];

                char buf[5];
                const char* pt;

                switch (c) {
                    case '\a':
                        pt = "\\a";
                        break;

                    case '\b':
                        pt = "\\b";
                        break;

                    case '\f':
                        pt = "\\f";
                        break;

                    case '\n':
                        pt = "\\n";
                        break;

                    case '\r':
                        pt = "\\r";
                        break;

                    case '\t':
                        pt = "\\t";
                        break;

                    case '\v':
                        pt = "\\v";
                        break;

                    case '\\':
                        pt = "\\\\";
                        break;

                    case '\"':
                        pt = "\\\"";
                        break;

                    case '\'':
                        pt = "\\\'";
                        break;

                    default:
                        if (c < ' ' || c > '~') {
                            buf[0] = '\\';
                            buf[1] = char(((c >> 6) & 0x3) + '0');
                            buf[2] = char(((c >> 3) & 0x7) + '0');
                            buf[3] = char(((c)&0x7) + '0');
                            buf[4] = '\0';
                        } else {
                            buf[0] = c;
                            buf[1] = '\0';
                        }

                        pt = buf;
                }

                tmp += pt;
            }
        } else {
            for (int i = 0; i < size; ++i) {
                char c = text[i];

                tmp.push_back((c < ' ' || c > '~') ? '.' : c);
            }
        }

        text = tmp.c_str();
        size = tmp.size();
    }

    int cnt = 0;

    if (size > maxSize) {
        size = maxSize;
    }

    if (minSize <= size) {
        dest.append(text, size);
    } else {
        if (!(type & alfafill)) {
            fillCharacter = ' ';
        }

        size_t destSize = dest.size();

        dest.append(minSize, fillCharacter);

        char* pt = &dest[destSize];

        if (type & leftJustify) {
            copy(pt, text, size);
        } else if (type & centerJustify) {
            size_t offset = (minSize - cnt - size) / 2;

            copy(pt + offset, text, size);
        } else {
            size_t offset = minSize - size;

            copy(pt + offset, text, size);
        }
    }
}

void tsioImplementation::outputNumber(std::string& dest,
                                      long long pNumber,
                                      int base,
                                      int size,
                                      int precision,
                                      unsigned type,
                                      char fillCharacter)
{
    if (precision < 0) {
        precision = 0;
    }

    char sign = 0;
    unsigned long long number;
    const size_t tmpBufSize = 134; // allow for 128 binary digits plus radix indicator and sign
    char tmpBuf[tmpBufSize];
    const char* digitPairs = "0001020304050607080910111213141516171819"
                             "2021222324252627282930313233343536373839"
                             "4041424344454647484950515253545556575859"
                             "6061626364656667686970717273747576777879"
                             "8081828384858687888990919293949596979899";

    char* bufPointer = tmpBuf + tmpBufSize;
    char* actualPointer = bufPointer;
    char* precisionPointer = bufPointer - precision;

    number = pNumber;

    if (type & signedNumber) {
        if (pNumber < 0) {
            sign = '-';
            number = -pNumber;
        } else if (type & plusIfPositive) {
            sign = '+';
        } else if (type & spaceIfPositive) {
            sign = ' ';
        }
    }

    if (precision != 0 || pNumber != 0) {
        switch (base) {
            case 2:
                do {
                    *(--bufPointer) = char((number & 1) + '0');
                    number >>= 1;
                } while (number != 0);

                while (bufPointer > precisionPointer) {
                    *(--bufPointer) = '0';
                }

                actualPointer = bufPointer;

                if (type & alternative && pNumber != 0) {
                    *(--bufPointer) = (type & upcase) ? 'B' : 'b';
                    *(--bufPointer) = '0';
                }

                break;

            case 8:
                do {
                    *(--bufPointer) = char((number & 7) + '0');
                    number >>= 3;
                } while (number != 0);

                while (bufPointer > precisionPointer) {
                    *(--bufPointer) = '0';
                }

                actualPointer = bufPointer;

                if (type & alternative) {
                    if (*bufPointer != '0') {
                        *(--bufPointer) = '0';
                    }
                }

                break;

            case 16: {
                const char* lowDigits = "0123456789abcdef";
                const char* upDigits = "0123456789ABCDEF";
                const char* digits = (type & upcase) ? upDigits : lowDigits;

                do {
                    *(--bufPointer) = digits[number & 15];
                    number >>= 4;
                } while (number != 0);

                while (bufPointer > precisionPointer) {
                    *(--bufPointer) = '0';
                }

                actualPointer = bufPointer;

                if (type & alternative && pNumber != 0) {
                    *(--bufPointer) = (type & upcase) ? 'X' : 'x';
                    *(--bufPointer) = '0';
                }
            }

            break;

            case 10:
                while (unsigned(number) != number) {
                    unsigned long long q = number / 100;
                    unsigned long long r = (number % 100) * 2;

                    *(--bufPointer) = digitPairs[r + 1];
                    *(--bufPointer) = digitPairs[r];

                    number = q;
                }

                unsigned uNumber = unsigned(number);

                while (uNumber >= 100) {
                    unsigned q = uNumber / 100;
                    unsigned r = (uNumber % 100) * 2;

                    *(--bufPointer) = digitPairs[r + 1];
                    *(--bufPointer) = digitPairs[r];

                    uNumber = q;
                }

                if (uNumber >= 10) {
                    unsigned long long r = uNumber * 2;

                    *(--bufPointer) = digitPairs[r + 1];
                    *(--bufPointer) = digitPairs[r];
                } else {
                    *(--bufPointer) = char(uNumber + '0');
                }

                while (bufPointer > precisionPointer) {
                    *(--bufPointer) = '0';
                }

                actualPointer = bufPointer;

                break;
        }
    } else {
        /// This is a hack to overcome an inconsistency between '%.x' and '%.o' formatting.
        if (base == 8 && (type & alternative)) {
            *(--bufPointer) = '0';
        }
    }

    if (sign != 0) {
        *(--bufPointer) = sign;
    }

    auto bytesNeeded = tmpBuf + tmpBufSize - bufPointer;
    auto actualDigits = tmpBuf + tmpBufSize - actualPointer;

    if (size <= bytesNeeded) {
            dest.append(bufPointer, bytesNeeded);
    } else {
        char fill = (type & (alfafill | numericfill)) ? fillCharacter : ' ';
        size_t destSize = dest.size();

        dest.append(size, fill);

        char* pt = &dest[destSize];

        if (type & leftJustify) {
            copy(pt, bufPointer, bytesNeeded);
        } else if (type & centerJustify) {
            size_t offset = (size - bytesNeeded) / 2;

            copy(pt + offset, bufPointer, bytesNeeded);
        } else if (type & numericfill) {
            auto prefixSize = actualPointer - bufPointer;

            copy(pt, bufPointer, prefixSize);
            copy(pt + size - actualDigits, actualPointer, actualDigits);
        } else {
            copy( pt + size - bytesNeeded, bufPointer, bytesNeeded);
        }
    }
}

tsioImplementation::FormatState::FormatState(const char*& format)
{
    parse(format);
}

void tsioImplementation::FormatState::parse(const char*& f)
{
    const char* format = f;

    unsigned char ch = *(format++);

    if (ch >= '1' && ch <= '9') {
        unsigned number = ch - '0';

        auto pt = format;

        for (; *pt >= '0' && *pt <= '9'; ++pt) {
            number = number * 10 + (*pt - '0');
        }

        if (*pt == '$') {
            position = number;
            format = pt + 1;
            ch = *(format++);
        } else {
            width = number;
            widthGiven = true;
            format = pt;
            ch = *(format++);
        }
    }

    if (!widthGiven) {
        char alfaFill = ' ';
        char numericFill = ' ';

        for (;;) {
            if (ch == '0') {
                type |= numericfill;
                numericFill = '0';
            } else if (ch == '-') {
                type |= leftJustify;
            } else if (ch == '^') {
                type |= centerJustify;
            } else if (ch == '+') {
                type |= plusIfPositive;
            } else if (ch == ' ') {
                type |= spaceIfPositive;
            } else if (ch == '#') {
                type |= alternative;
            } else if (ch == '\'') {
                type |= numericfill;

                if (*format != 0) {
                    numericFill = *(format++);
                }
            } else if (ch == '\"') {
                type |= alfafill;

                if (*format != 0) {
                    alfaFill = *(format++);
                }
            } else {
                break;
            }

            ch = *(format++);
        }

        if ((type & (plusIfPositive | spaceIfPositive)) == (plusIfPositive | spaceIfPositive)) {
            type &= ~spaceIfPositive;
        }

        if ((type & (leftJustify | numericfill)) == (leftJustify | numericfill)) {
            type &= ~numericfill;
        }

        if ((type & (centerJustify | numericfill)) == (centerJustify | numericfill)) {
            type &= ~numericfill;
        }

        if ((type & (alfafill | numericfill)) == (alfafill | numericfill)) {
            type &= ~alfafill;
        }

        if (type & numericfill) {
            fillCharacter = numericFill;
        } else if (type & alfafill) {
            fillCharacter = alfaFill;
        }

        if (ch == '*') {
            widthDynamic = true;
            widthGiven = true;
            active = true;
            ch = *(format++);

            if (unsigned(ch - '0') < 9) {
                widthPosition = unsigned(ch - '0');
                ch = *(format++);

                while (unsigned(ch - '0') < 9) {
                    widthPosition = widthPosition * 10 + (unsigned(ch - '0'));
                    ch = *(format++);
                }

                if (ch == '$') {
                    ch = *(format++);
                }
            }
        } else {
            if (unsigned(ch - '0') < 9) {
                widthGiven = true;
                width = unsigned(ch - '0');
                ch = *(format++);

                while (unsigned(ch - '0') < 9) {
                    width = width * 10 + (unsigned(ch - '0'));
                    ch = *(format++);
                }
            }
        }
    }

    if (ch == '.') {
        ch = *(format++);
        precisionGiven = true;

        if (ch == '*') {
            precisionDynamic = true;
            active = true;
            ch = *(format++);

            if (unsigned(ch - '0') < 9) {
                precisionPosition = unsigned(ch - '0');
                ch = *(format++);

                while (unsigned(ch - '0') < 9) {
                    precisionPosition = precisionPosition * 10 + (unsigned(ch - '0'));
                    ch = *(format++);
                }

                if (ch == '$') {
                    ch = *(format++);
                }
            }
        }
        if (unsigned(ch - '0') < 9) {
            precision = unsigned(ch - '0');

            ch = *(format++);

            while (unsigned(ch - '0') < 9) {
                precision = precision * 10 + (unsigned(ch - '0'));
                ch = *(format++);
            }
        }
    }

    while (ch == 'h' || ch == 'j' || ch == 'l' || ch == 'L' || ch == 't' || ch == 'z') {
        ch = *(format++);
    }

    formatSpecifier = ch;
    if (ch == 0) {
        f = format - 1;
    } else {
        f = format;
    }
}

// unparse generates a standard format.  Extensions are mostly not generated.
const char* tsioImplementation::FormatState::unParse() const
{
    buf[30] = 0;
    char* pt = buf + 30;

    *(--pt) = formatSpecifier;

    if (precisionGiven) {
        unsigned tmp = precision;

        do {
            unsigned q = tmp / 10;
            unsigned r = tmp - q * 10;

            *(--pt) = char(r + '0');
            tmp = q;
        } while (tmp != 0);

        *(--pt) = '.';
    }

    if (widthGiven) {
        unsigned tmp = width;

        do {
            unsigned q = tmp / 10;
            unsigned r = tmp - q * 10;

            *(--pt) = char(r + '0');
            tmp = q;
        } while (tmp != 0);
    }

    if (type != 0) {
        if (type & numericfill) {
            *(--pt) = '0';
        }

        if (type & plusIfPositive) {
            *(--pt) = '+';
        }

        if (type & spaceIfPositive) {
            *(--pt) = ' ';
        }

        if (type & leftJustify) {
            *(--pt) = '-';
        }

        if (type & alternative) {
            *(--pt) = '#';
        }
    }

    *(--pt) = '%';

    return pt;
}

void tsioImplementation::Format::copyToFormat()
{
    const char* f = format;
    const char* pt0 = f;

    while (*f != 0) {
        if (*f == '%') {
            if (f != pt0) {
                dest.append(pt0, f - pt0);
            }

            f++;
            pt0 = f;

            if (*f == '}') {
                if (repeat()) {
                    pt0 = format;
                    f = pt0 - 1;
                } else {
                    pt0 = f + 1;
                }
            } else if (*f != '%') {
                format = f;
                return;
            }
        }

        f++;
    }

    if (f != pt0) {
        dest.append(pt0, f - pt0);
    }

    format = f;
}

void tsioImplementation::Format::skipToFormat()
{
    while (*format != 0) {
        if (*format == '%') {
            format++;

            if (*format != '%') {
                return;
            }
        }

        format++;
    }
}

void tsioImplementation::Format::parse()
{
    state.reset();
    copyToFormat();
    state.parse(format);

    if (state.formatSpecifier == '[') {
        state.prefix = format;
        skipToFormat();
        state.prefixSize = unsigned(format - state.prefix - 1);

        state.parse(format);

        state.suffix = format;
        skipToFormat();
        state.postfixSize = unsigned(format - state.suffix - 1);

        if (*format != ']') {
            std::cerr << "TSIO: Missing '%]'.\n";
        } else {
            format++;
        }

        state.isContainerFormat = true;
    }
}

void tsio::fmt::initialize(const char* format)
{
    if (format == 0) {
        return;
    }

    if (*format == '%') {
        format++;
    }

    state.parse(format);
}

std::ostream& tsio::fmt::operator()(std::ostream& out) const
{
    using namespace tsioImplementation;

    out.width(0);
    out.precision(6);
    out.fill(' ');

    out.unsetf(std::ios::adjustfield | std::ios::basefield | std::ios::floatfield | std::ios::showbase |
               std::ios::boolalpha | std::ios::showpoint | std::ios::showpos | std::ios::uppercase);

    if (state.formatSpecifier == 0) {
        return out;
    }

    if (state.type & leftJustify) {
        out.setf(std::ios::left, std::ios::adjustfield);
    }

    if (state.type & numericfill) {
        out.fill(state.fillCharacter);
        out.setf(std::ios::internal, std::ios::adjustfield);
    }

    if (state.type & alfafill) {
        out.fill(state.fillCharacter);
    }

    if (state.type & plusIfPositive) {
        out.setf(std::ios::showpos);
    }

    if (state.type & alternative) {
        out.setf(std::ios::showpoint | std::ios::showbase);
    }

    // spaceIfPositive can not be implemented on streams
    if (state.widthGiven) {
        out.width(state.width);
    }

    if (state.precisionGiven) {
        out.precision(state.precision);
    }

    switch (state.formatSpecifier) {
        case 'd':
        case 'i':
        case 'u':
            break;

        case 'o':
            out.setf(std::ios::oct, std::ios::basefield);
            break;

        case 'X':
            out.setf(std::ios::uppercase);
            // Fallthru
        case 'x':
        case 'p':
            out.setf(std::ios::hex, std::ios::basefield);
            break;

        case 's':
            out.setf(std::ios::boolalpha);
            break;

        case 'E':
            out.setf(std::ios::uppercase);
            // Fallthru
        case 'e':
            out.setf(std::ios::scientific, std::ios::floatfield);
            break;

        case 'F':
            out.setf(std::ios::uppercase);
            // Fallthru
        case 'f':
            out.setf(std::ios::fixed, std::ios::floatfield);
            break;

        case 'G':
            out.setf(std::ios::uppercase);
            // Fallthru
        case 'g':
            out.unsetf(std::ios::floatfield);
            break;

        case 'A':
            out.setf(std::ios::uppercase);
            // Fallthru
        case 'a':
            out.setf(std::ios_base::fixed | std::ios_base::scientific, std::ios_base::floatfield);
            break;
    }

    return out;
}

void tsioImplementation::printfDetail(std::string& dest, const FormatState& state, const std::string& value)
{
    char format = state.formatSpecifier;

    switch (format) {
        case 's':
            outputString(dest,
                         value.c_str(),
                         value.size(),
                         state.width,
                         state.precisionGiven ? state.precision : std::numeric_limits<int>::max(),
                         state.type,
                         state.fillCharacter);

            break;

        case 'S':
            outputString(dest,
                         value.c_str(),
                         value.size(),
                         state.width,
                         state.precisionGiven ? state.precision : std::numeric_limits<int>::max(),
                         state.type | nice,
                         state.fillCharacter);

            break;

        default:
            std::cerr << "TSIO: Invalid format '" << format << "' for std::string value" << std::endl;
    }
}

void tsioImplementation::printfDetail(std::string& dest, const FormatState& state, const char* value)
{
    char format = state.formatSpecifier;
    uintptr_t pValue = uintptr_t(value);

    switch (format) {
        case 'p':
            outputNumber(dest,
                         pValue,
                         16,
                         state.width,
                         state.precision,
                         state.type | alternative,
                         state.fillCharacter);

            break;

        case 's':
            outputString(dest,
                         value,
                         state.width,
                         state.precisionGiven ? state.precision : std::numeric_limits<int>::max(),
                         state.type,
                         state.fillCharacter);

            break;

        case 'S':
            outputString(dest,
                         value,
                         state.width,
                         state.precisionGiven ? state.precision : std::numeric_limits<int>::max(),
                         state.type | nice,
                         state.fillCharacter);

            break;

        default:
            printfDetail(dest, state, static_cast<uintptr_t>(pValue));
    }
}

void tsioImplementation::printfDetail(std::string& dest, const FormatState& state, double value)
{
    char format = state.formatSpecifier;

    switch (format) {
        case 's':
        case 'a':
        case 'A':
        case 'e':
        case 'E':
        case 'f':
        case 'F':
        case 'g':
        case 'G': {
            const char* f;

            if (format == 's') {
                FormatState newFlags(state);

                newFlags.formatSpecifier = 'g';
                f = newFlags.unParse();
            } else {
                f = state.unParse();
            }

            static char* pt = nullptr;
            static size_t allocatedSize = 0;

            int s = snprintf(pt, allocatedSize, f, value);

            if (s < 0) {
                return;
            }

            if (s >= int(allocatedSize)) {
                allocatedSize = s * 2 + 1;
                pt = static_cast<char*>(realloc(pt, allocatedSize));
                sprintf(pt, f, value);
            }

            dest.append(pt, s);
        }

        break;

        default:
            std::cerr << "TSIO: Invalid format '" << format << "' for floating point value" << std::endl;
    }
}

void tsioImplementation::printfDetail(std::string& dest, const FormatState& state, float value)
{
    printfDetail(dest, state, double(value));
}

void tsioImplementation::printfDetail(std::string& dest, const FormatState& state, bool value)
{
    char format = state.formatSpecifier;

    if (format == 's') {
        const char* pt = value ? "true" : "false";

        printfDetail(dest, state, pt);
    } else {
        printfDetail(dest, state, static_cast<long long>(value));
    }
}

void tsioImplementation::skipAfter(const char*& format, char startChar, char endChar)
{
    const char* f = format;
    size_t count = 0;

    while (*f != 0) {
        char ch = *(f++);

        if (ch == '%') {
            ch = *(f++);

            while (ch >= '0' && ch <= '9') {
                ch = *(f++);
            }

            if (ch == startChar) {
                count++;
            } else if (ch == endChar) {
                if (count-- == 0) {
                    format = f;
                    return;
                }
            }
        }
    }

    std::cerr << "TSIO: Missing '%" << endChar << "'\n";
    format = f;
}
