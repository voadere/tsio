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

TSIO_NEVER_INLINE void tsioImplementation::Buffer::resize(size_t newSize)
{
    size_t newLen = mLen;

    while (newLen < newSize) {
        newLen += newLen / 2;
    }

    newLen = ((newLen + 15) / 16) * 16;

    if (mData == mShortData) {
        mData = static_cast<char*>(malloc(newLen));

        std::copy(mShortData, mShortData + mEod, mData);
    } else {
        mData = static_cast<char*>(realloc(mData, newLen));
    }

    mLen = newLen;
}

static void outputString(tsioImplementation::Buffer& dest,
                         const char* text,
                         size_t s,
                         int minSize,
                         int maxSize,
                         unsigned type,
                         char fillCharacter)
{
    using namespace tsioImplementation;

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
                            buf[3] = char((c & 0x7) + '0');
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
        size_t fillSize = minSize - size;

        dest.widen(minSize);

        char* pt = dest.data() + destSize;

        if (type & leftJustify) {
            pt = copy(pt, text, size);
            fill(pt, fillCharacter, fillSize);
        } else if (type & centerJustify) {
            size_t offset = fillSize / 2;
            size_t rest = minSize - offset;

            pt = fill(pt, fillCharacter, offset);
            pt = copy(pt, text, size);
            fill(pt, fillCharacter, rest);
        } else {
            pt = fill(pt, fillCharacter, fillSize);
            copy(pt, text, size);
        }
    }
}

inline void outputString(tsioImplementation::Buffer& dest,
                         const char* text,
                         int minSize,
                         int maxSize,
                         unsigned type,
                         char fillCharacter)
{
    outputString(dest, text, strlen(text), minSize, maxSize, type, fillCharacter);
}

template <int base, bool isSigned = false>
static void outputNumber(tsioImplementation::Format& format,
                         long long pNumber,
                         unsigned type)
{
    using namespace tsioImplementation;

    auto& state = format.nextNode->state;
    int size = state.width;
    unsigned long long number = pNumber;
    const size_t bufSize = 134; // allow for 128 binary digits plus radix indicator and sign
    char buf[bufSize];
    int precision = state.precisionGiven() ? state.precision : 1;

    if (precision < 0) {
        precision = 0;
    }

    char* actualPointer = buf + bufSize;
    char* precisionPointer = actualPointer - precision;
    char* prefixPointer;
    char sign = 0;

    if (isSigned) {
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
                    *(--actualPointer) = char((number & 1) + '0');
                    number >>= 1;
                } while (number != 0);

                while (actualPointer > precisionPointer) {
                    *(--actualPointer) = '0';
                }

                prefixPointer = actualPointer;

                if (type & alternative && pNumber != 0) {
                    *(--prefixPointer) = (type & upcase) ? 'B' : 'b';
                    *(--prefixPointer) = '0';
                }

                break;

            case 8:
                do {
                    *(--actualPointer) = char((number & 7) + '0');
                    number >>= 3;
                } while (number != 0);

                while (actualPointer > precisionPointer) {
                    *(--actualPointer) = '0';
                }

                prefixPointer = actualPointer;

                if (type & alternative) {
                    if (*actualPointer != '0') {
                        *(--prefixPointer) = '0';
                    }
                }

                break;

            case 16: {
                const char* lowDigits = "0123456789abcdef";
                const char* upDigits = "0123456789ABCDEF";
                const char* digits = (type & upcase) ? upDigits : lowDigits;

                do {
                    *(--actualPointer) = digits[number & 15];
                    number >>= 4;
                } while (number != 0);

                while (actualPointer > precisionPointer) {
                    *(--actualPointer) = '0';
                }

                prefixPointer = actualPointer;

                if (type & alternative && pNumber != 0) {
                    *(--prefixPointer) = (type & upcase) ? 'X' : 'x';
                    *(--prefixPointer) = '0';
                }
            }

            break;

            case 10: {
                const char* digitPairs = "0001020304050607080910111213141516171819"
                                         "2021222324252627282930313233343536373839"
                                         "4041424344454647484950515253545556575859"
                                         "6061626364656667686970717273747576777879"
                                         "8081828384858687888990919293949596979899";

                while (unsigned(number) != number) {
                    unsigned long long q = number / 100;
                    unsigned long long r = (number % 100) * 2;

                    *(--actualPointer) = digitPairs[r + 1];
                    *(--actualPointer) = digitPairs[r];

                    number = q;
                }

                unsigned uNumber = unsigned(number);

                while (uNumber >= 100) {
                    unsigned q = uNumber / 100;
                    unsigned r = (uNumber % 100) * 2;

                    *(--actualPointer) = digitPairs[r + 1];
                    *(--actualPointer) = digitPairs[r];

                    uNumber = q;
                }

                if (uNumber >= 10) {
                    unsigned long long r = uNumber * 2;

                    *(--actualPointer) = digitPairs[r + 1];
                    *(--actualPointer) = digitPairs[r];
                } else {
                    *(--actualPointer) = char(uNumber + '0');
                }

                while (actualPointer > precisionPointer) {
                    *(--actualPointer) = '0';
                }

                prefixPointer = actualPointer;

                break;
            }
        }
    } else {
        /// This is a hack to overcome an inconsistency between '%.x' and '%.o' formatting.
        prefixPointer = actualPointer;

        if (base == 8 && (type & alternative)) {
            *(--prefixPointer) = '0';
        }
    }

    if (sign != 0) {
        *(--prefixPointer) = sign;
    }

    auto& dest = format.dest;
    size_t bytesNeeded = buf + bufSize - prefixPointer;

    if (size_t(size) <= bytesNeeded) {
        dest.append(prefixPointer, bytesNeeded);
    } else {
        char fillCharacter;

        if ((type & (alfafill | numericfill)) == 0) {
            fillCharacter = ' ';
        } else if ((type & (numericfill | precisionGiven)) == (numericfill | precisionGiven)) {
            type &= ~numericfill;
            fillCharacter = ' ';
        } else {
            fillCharacter = state.fillCharacter;
        }

        size_t destSize = dest.size();

        dest.widen(size);

        char* pt = dest.data() + destSize;
        size_t fillSize = size - bytesNeeded;

        if ((type & (leftJustify | centerJustify | numericfill)) == 0) {
            pt = fill(pt, fillCharacter, fillSize);
            copy(pt, prefixPointer, bytesNeeded);
        } else if (type & leftJustify) {
            pt = copy(pt, prefixPointer, bytesNeeded);
            fill(pt, fillCharacter, fillSize);
        } else if (type & centerJustify) {
            size_t offset = fillSize / 2;
            size_t rest = size - offset;

            pt = fill(pt, fillCharacter, offset);
            pt = copy(pt, prefixPointer, bytesNeeded);
            fill(pt, fillCharacter, rest);
        } else { // numericFill
            size_t prefixSize = actualPointer - prefixPointer;
            auto actualDigits = bytesNeeded - prefixSize;

            pt = copy(pt, prefixPointer, prefixSize);
            pt = fill(pt, fillCharacter, fillSize);
            copy(pt, actualPointer, actualDigits);
        }
    }
}

void tsioImplementation::outputPointer(Format& format,
                                       uintptr_t pNumber)
{
    auto& state = format.nextNode->state;

    outputNumber<16>(format, pNumber, state.type | alternative);
}

void tsioImplementation::FormatState::parse(const char*& f)
{
    const char* format = f;
    bool startsWithWidth = false;

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
            setWidthGiven();
            startsWithWidth = true;
            format = pt;
            ch = *(format++);
        }
    }

    if (!startsWithWidth) {
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

        if ((type & (plusIfPositive | spaceIfPositive  | numericfill | alfafill)) != 0) {
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
        }

        if (ch == '*') {
            setWidthDynamic();
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
                setWidthGiven();
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
        setPrecisionGiven();

        if (ch == '*') {
            setPrecisionDynamic();
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
        setSpecial();
    } else {
        if (ch == '%' || ch == 'T' || ch == '{' || ch == '}' || ch == 'N') {
            setSpecial();
        }

        f = format;
    }
}

const char* tsioImplementation::FormatState::unParseForFloat(bool longDouble) const
{
    // the string is built back to front.
    buf[30] = 0;
    char* pt = buf + 30;

    *(--pt) = (formatSpecifier == 's') ? 'g' : formatSpecifier;

    if (longDouble) {
        *(--pt) = 'L';
    }

    if (precisionGiven()) {
        if (widthDynamic()) {
            *(--pt) = '*';
        } else {
            unsigned tmp = precision;

            do {
                unsigned q = tmp / 10;
                unsigned r = tmp - q * 10;

                *(--pt) = char(r + '0');
                tmp = q;
            } while (tmp != 0);
        }

        *(--pt) = '.';
    }

    // width is defaulted
    // Only some flags are returned.

    if (type != 0) {
        if (type & plusIfPositive) {
            *(--pt) = '+';
        }

        if (type & spaceIfPositive) {
            *(--pt) = ' ';
        }

        if (type & alternative) {
            *(--pt) = '#';
        }
    }

    *(--pt) = '%';

    return pt;
}

void tsioImplementation::Format::showErrorContext(FormatNode* node) const
{
    std::cerr << "         at \"" << wholeFormat << "\"\n";

    size_t offset;

    if (node == nullptr) {
        offset = strlen(wholeFormat);
    } else {
        offset = node->format - wholeFormat + node->state.prefixSize;
    }

    std::cerr << std::setw(offset + 12 + 2) << '^' << std::endl;
}

tsioImplementation::FormatNode* tsioImplementation::Format::getNode()
{
    if (chuncks->index == chuncks->chunckSize - 1) {
        auto pt = new FormatNodes;

        pt->next = chuncks;
        chuncks = pt;
    }

    auto result = &chuncks->nodes[chuncks->index++];
    result->reset();
    result->format = format;
    return result;
}

tsioImplementation::FormatNode* tsioImplementation::Format::buildTree()
{

    if (*format == 0) {
        return nullptr;
    }

    FormatNode* node = getNode();
    FormatNode* result = node;
    FormatNode* next = node;

    for (;;) {
        auto& state = node->state;
        const char* fmt = format;

        state.start = fmt;
        state.prefix = fmt;
        while (*fmt != 0 && *(fmt++) != '%') {
            // nop
        }

        if (*fmt == 0) {
            state.prefixSize = unsigned(fmt - state.prefix);
            state.setSpecial();
            state.size = fmt - format;
            break;
        }

        state.prefixSize = unsigned(fmt - state.prefix - 1);

        state.parse(fmt);

        state.size = fmt - format;

        format = fmt;
        if (state.position != 0 || state.widthPosition != 0 || state.precisionPosition != 0) {
            positional = true;
        }

        auto spec = state.formatSpecifier;

        if (spec == '}' || spec == ']' || spec == '>') {
            break;
        }

        if (spec == '{' || spec == '[') {
            node->child = buildTree();
        } else if (spec == '<') {
            bool parentPositional = positional;

            positional = false;

            node->child = buildTree();

            if (positional) {
                state.setPositionalChildren();
            }

            positional = parentPositional;
        }

        node = getNode();
        next->next = node;
        next = node;
    }

    return result;
}

void tsioImplementation::Format::tabTo(unsigned column, bool absolute)
{
    const char* nlPointer = dest.data() + dest.size();

    while (nlPointer > dest.data() && nlPointer[-1] != '\n') {
        --nlPointer;
    }

    size_t currentColumn = dest.data() + dest.size() - nlPointer;

    if (absolute) {
        column--;

        if (currentColumn < column) {
            dest.append(column - currentColumn, ' ');
        } else {
            dest.push_back('\n');
            dest.append(column, ' ');
        }
    } else {
        auto delta = column - (currentColumn % column);
        if (delta == 0) {
            delta = column;
        }

        dest.append(delta, ' ');
    }
}

void tsioImplementation::Format::handleSpecialNodes(FormatNode*& node)
{
    auto& state = node->state;
    auto spec = state.formatSpecifier;

    if (state.prefixSize != 0) {
        dest.append(state.prefix, state.prefixSize);
    }

    if (spec == 0) {
        node = node->next;
    } if (spec == '%') {
        dest.push_back('%');
        node = node->next;
    } else if (spec == 'T') {
        tabTo(state.width, state.type & alternative);
        node = node->next;
    } else if (spec == 'N') {
        if (indexStack.empty()) {
            error("%N format is only valid inside rpeatimg, loop and sequence formats");
        } else {
            auto index = indexStack.back();

            if (!(state.type & alternative)) {
                index++;
            }

            auto tmpNode = nextNode;

            nextNode = node;
            outputNumber<10>(*this, index, state.type);
            nextNode = tmpNode;
        }


        node = node->next;
    } else if (spec == '{') {
        auto child = node->child;

        if (child->next == nullptr && child->state.formatSpecifier == '}') {
            for (unsigned i = 0, c = state.width; i < c; ++i) {
                dest.append(child->state.prefix, child->state.prefixSize);
            }

            node = node->next;
        } else {
            pushRepeat(node, state.width);
            indexStack.push_back(0);
            node = child;
        }
    } else if (spec == '}') {
        if (!repeatStack.empty()) {
            auto& element = repeatStack.back();

            if ((element.count--) == 0) {
                node = element.node->next;
                repeatStack.pop_back();
                indexStack.pop_back();
            } else {
                node = element.node->child;
                indexStack.back()++;
            }
        } else {
            node = node->next;
        }
    }
}

tsioImplementation::FormatNode* tsioImplementation::Format::getNextSibling(FormatNode* node, bool first)
{
    if (node == nullptr) {
        return node;
    }

    if (!first) {
        node = node->next;
    }

    while (node != nullptr && node->state.nonDynamicSpecial()) {
        handleSpecialNodes(node);
    }

    return node;
}

tsioImplementation::FormatNode* tsioImplementation::Format::getChild(FormatNode* node)
{
    auto child = node->child;
    auto& state = child->state;

    if (state.nonDynamicSpecial()) {
        return getNextSibling(child, true);
    }

    return child;
}

void tsioImplementation::Format::getNextNode(bool first)
{
    if (nextNode == nullptr) {
        return;
    }

    if (!first) {
        nextNode = nextNode->next;
    }

    while (nextNode != nullptr && nextNode->state.nonDynamicSpecial()) {
        handleSpecialNodes(nextNode);
    }
}

void tsioImplementation::Format::printTree(std::ostream& os, FormatNode* node, unsigned indent)
{
    const auto& state = node->state;

    for (unsigned i = 0; i < indent; ++i) {
        os << "  ";
    }

    os << '"';
    os.write(state.start, state.size);
    os << '"';

    os << '\n';

    auto child = node->child;

    while (child != nullptr) {
        printTree(os, child, indent + 1);

        child = child->next;
    }
}

void tsioImplementation::Format::dump()
{
    for (auto node = nextNode; node != 0; node = node->next) {
        printTree(std::cout, node, 0);
    }
}

void tsio::fmt::initialize(const char* format)
{
    state.reset();

    if (format == 0) {
        return;
    }

    if (*format == '%') {
        format++;
    }

    state.parse(format);
}

void tsioImplementation::printfDetail(Format& format,
                                      long long sValue,
                                      unsigned long long uValue,
                                      bool isSigned)
{
    auto& state = format.nextNode->state;
    auto& dest = format.dest;
    char spec = state.formatSpecifier;
    char fillCharacter = state.fillCharacter;

    auto type = state.type;

    // let's favor the most used formats
    if (spec == 'd' || spec == 'i') {
        if (sValue > 0 && (type & (plusIfPositive | spaceIfPositive)) == 0) {
            outputNumber<10>(format, uValue, type);
            return;
        }

        outputNumber<10, true>(format, sValue, type);
        return;
    } else if (spec == 'u') {outputNumber<10>(
                format, uValue, type);
        return;
    }

    switch (spec) {
        case 'X':
            outputNumber<16>(format, uValue, type | upcase);
            return;

        case 'B':
            outputNumber<2>(format, uValue, type | upcase);
            return;

        case 'C': {
            char c = char(sValue);
            outputString(dest,
                         &c,
                         1,
                         state.width,
                         state.precisionGiven() ? (state.precision > 0 ? state.precision : 1)
                                                : std::numeric_limits<int>::max(),
                         type | nice,
                         fillCharacter);
            return;
        }

        case 'b':
            outputNumber<2>(format, uValue, type);
            return;

        case 'c': {
            char c = char(sValue);
            outputString(dest,
                         &c,
                         1,
                         state.width,
                         state.precisionGiven() ? (state.precision > 0 ? state.precision : 1)
                                                : std::numeric_limits<int>::max(),
                         type,
                         fillCharacter);
            return;
        }

        case 'n':
            format.error("Did you forget to specify the parameter for '%n' by pointer");
            return;

        case 'o':
            outputNumber<8>(format, uValue, type);
            return;

        case 's':
            if (isSigned) {
                outputNumber<10, true>(format, sValue, type);
            } else {
                outputNumber<10>(format, uValue, type);
            }

            return;

        case 'x':
            outputNumber<16>(format, uValue, type);
            return;
    }

    format.error("Invalid format '", spec, "' for integeral value");
}

void tsioImplementation::printfDetail(Format& format, const std::string& value)
{
    auto& state = format.nextNode->state;
    auto& dest = format.dest;
    char spec = state.formatSpecifier;

    switch (spec) {
        case 's':
            outputString(dest,
                         value.c_str(),
                         value.size(),
                         state.width,
                         state.precisionGiven() ? state.precision : std::numeric_limits<int>::max(),
                         state.type,
                         state.fillCharacter);

            break;

        case 'S':
            outputString(dest,
                         value.c_str(),
                         value.size(),
                         state.width,
                         state.precisionGiven() ? state.precision : std::numeric_limits<int>::max(),
                         state.type | nice,
                         state.fillCharacter);

            break;

        default:
            format.error("Invalid format '", spec, "' for std::string value");
    }
}

void tsioImplementation::printfDetail(Format& format, const char* value)
{
    auto& state = format.nextNode->state;
    auto& dest = format.dest;
    char spec = state.formatSpecifier;
    uintptr_t pValue = uintptr_t(value);

    switch (spec) {
        case 'p':
            outputPointer(format, pValue);

            break;

        case 's':
            outputString(dest,
                         value,
                         state.width,
                         state.precisionGiven() ? state.precision : std::numeric_limits<int>::max(),
                         state.type,
                         state.fillCharacter);

            break;

        case 'S':
            outputString(dest,
                         value,
                         state.width,
                         state.precisionGiven() ? state.precision : std::numeric_limits<int>::max(),
                         state.type | nice,
                         state.fillCharacter);

            break;

        default:
            printfDetail(format, static_cast<uintptr_t>(pValue));
    }
}

static void outputFloatTmp(tsioImplementation::Format& format,
        const tsioImplementation::Buffer& tmp)
{
    using namespace tsioImplementation;

    auto& state = format.nextNode->state;
    auto& dest = format.dest;
    size_t bytesNeeded = tmp.size();

    if (!state.widthGiven() || bytesNeeded > state.width) {
        dest.append(tmp.data(), bytesNeeded);
    } else {
        unsigned type = state.type;
        size_t size = state.width;
        char fillChar = (type & (alfafill | numericfill)) ? state.fillCharacter : ' ';
        size_t destSize = dest.size();
        size_t fillSize = size - bytesNeeded;

        dest.widen(size);

        char* pt = dest.data() + destSize;

        if (type & leftJustify) {
            pt = copy(pt, tmp.data(), tmp.size());
            fill(pt, fillChar, fillSize);
        } else if (type & centerJustify) {
            size_t offset = fillSize / 2;
            size_t rest = size - offset;

            pt = fill(pt, fillChar, offset);
            pt = copy(pt, tmp.data(), tmp.size());
            fill(pt, fillChar, rest);
        } else if (type & numericfill) {
            const char* prefix = tmp.data();
            const char* actualPointer = prefix;

            if (*actualPointer == ' ' || *actualPointer == '+' || *actualPointer == '-') {
                actualPointer++;
            }

            if (*actualPointer == '0' && (actualPointer[1] == 'x' || actualPointer[1] == 'X')) {
                actualPointer += 2;
            }

            size_t prefixSize = actualPointer - tmp.data();
            size_t actualDigits = tmp.size() - prefixSize;

            pt = copy(pt, prefix, prefixSize);
            pt = fill(pt, fillChar, fillSize);
            copy(pt, actualPointer, actualDigits);
        } else {
            pt = fill(pt, fillChar, fillSize);
            pt = copy(pt, tmp.data(), tmp.size());
        }
    }
}

void tsioImplementation::printfDetail(Format& format, double value)
{
    auto& state = format.nextNode->state;
    auto& dest = format.dest;
    char spec = state.formatSpecifier;

    switch (spec) {
        case 's':
        case 'a':
        case 'A':
        case 'e':
        case 'E':
        case 'f':
        case 'F':
        case 'g':
        case 'G': {
            const char* f = state.unParseForFloat(false);
            Buffer tmp;
            char* pt = tmp.data();
            size_t capacity = tmp.capacity();

            int s = snprintf(pt, capacity, f, value);

            if (s < 0) {
                return;
            }

            tmp.widen(s);

            if (s > int(capacity)) {
                pt = dest.data();
                sprintf(pt, f, value);
            }

            outputFloatTmp(format, tmp);
        }

        break;

        default:
            format.error("Invalid format '", spec, "' for floating point value");
    }
}

void tsioImplementation::printfDetail(Format& format, float value)
{
    printfDetail(format, double(value));
}

void tsioImplementation::printfDetail(Format& format, long double value)
{
    auto& state = format.nextNode->state;
    auto& dest = format.dest;
    char spec = state.formatSpecifier;

    switch (spec) {
        case 's':
        case 'a':
        case 'A':
        case 'e':
        case 'E':
        case 'f':
        case 'F':
        case 'g':
        case 'G': {
            const char* f = state.unParseForFloat(true);
            Buffer tmp;
            char* pt = tmp.data();
            size_t capacity = tmp.capacity();

            int s = snprintf(pt, capacity, f, value);

            if (s < 0) {
                return;
            }

            tmp.widen(s);

            if (s > int(capacity)) {
                pt = dest.data();
                sprintf(pt, f, value);
            }

            outputFloatTmp(format, tmp);
        }

        break;

        default:
            format.error("Invalid format '", spec, "' for floating point value");
    }
}

void tsioImplementation::printfDetail(Format& format, bool value)
{
    char spec = format.nextNode->state.formatSpecifier;

    if (spec == 's') {
        const char* pt = value ? "true" : "false";

        printfDetail(format, pt);
    } else {
        printfDetail(format, static_cast<long long>(value));
    }
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
    if (state.widthGiven()) {
        out.width(state.width);
    }

    if (state.precisionGiven()) {
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

tsio::CFormat::CFormat(const char* f)
{
    size_t size = strlen(f);

    formatCache = static_cast<char*>(malloc(size + 1));
    memcpy(formatCache, f, size + 1);
    format.format = formatCache;
    format.wholeFormat = formatCache;

    format.nextNode = format.buildTree();
}

tsio::CFormat::~CFormat()
{
    free(formatCache);
}
