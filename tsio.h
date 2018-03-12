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

#ifndef TSIO_H
#define TSIO_H

#include <array>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <malloc.h>
#include <string>
#include <vector>

#if defined(_MSC_VER)
#define TSIO_ALWAYS_INLINE __forceinline
#define TSIO_NEVER_INLINE __declspec(noinline)
#else
#define TSIO_ALWAYS_INLINE __attribute__((always_inline))
#define TSIO_NEVER_INLINE __attribute__ ((noinline))
#endif

namespace tsioImplementation
{
enum TypeEnum {
    numericfill = 1,
    alfafill = numericfill << 1,
    plusIfPositive = alfafill << 1,
    spaceIfPositive = plusIfPositive << 1,
    leftJustify = spaceIfPositive << 1,
    centerJustify = leftJustify << 1,
    alternative = centerJustify << 1,
    widthDynamic = alternative << 1,
    precisionDynamic = widthDynamic << 1,
    widthGiven = precisionDynamic << 1,
    precisionGiven = widthGiven << 1,
    positionalChildren = precisionGiven << 1,
    upcase = positionalChildren << 1,
    nice = upcase << 1,
    special = nice << 1
};

inline TSIO_ALWAYS_INLINE char* copy(char* dest, const char* src, unsigned count)
{
    switch(count) {
        case 4: dest[3] = src[3];
        case 3: dest[2] = src[2];
        case 2: dest[1] = src[1];
        case 1: dest[0] = src[0];
        case 0: break;
        default: memcpy(dest, src, count);
    }

    return dest + count;
}

inline TSIO_ALWAYS_INLINE char* fill(char* dest, char c, unsigned count)
{
    switch(count) {
        case 4: dest[3] = c;
        case 3: dest[2] = c;
        case 2: dest[1] = c;
        case 1: dest[0] = c;
        case 0: break;
        default: memset(dest, c, count);
    }

    return dest + count;
}

class Buffer
{
    public:
        Buffer() = default;

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        ~Buffer() {
            if (mData != mShortData) {
                free(mData);
            }
        }

        void clear()
        {
            mEod = 0;
        }

        size_t size() const {
            return mEod;
        }

        size_t capacity() const
        {
            return mLen;
        }

        char* data() const
        {
            return mData;
        }

        void widen(size_t count)
        {
            if (count > 0) {
                if (mEod + count > mLen) {
                    resize(mEod + count);
                }

                mEod += count;
            }
        }

        void append(const char* value, size_t count)
        {
            if (count > 0) {
                if (mEod + count > mLen) {
                    resize(mEod + count);
                }

                copy(mData + mEod, value, count);
                mEod += count;
            }
        }

        void append(size_t count, char value)
        {
            if (count > 0) {
                if (mEod + count > mLen) {
                    resize(mEod + count);
                }

                fill(mData + mEod, value, count);
                mEod += count;
            }
        }

        void push_back(char value)
        {
            if (mEod + 1 > mLen) {
                resize(mEod + 1);
            }

            mData[mEod++] = value;
        }

    private:
        void resize(size_t newSize);

        static const size_t shortSize = 1024;
        size_t mLen = shortSize;
        size_t mEod = 0;
        char*  mData = mShortData;
        char   mShortData[shortSize];

};

struct FormatState
{
    FormatState() = default;

    void parse(const char*& format);

    const char* unParse() const;

    bool widthGiven() const
    {
        return type & TypeEnum::widthGiven;
    }

    bool precisionGiven() const
    {
        return type & TypeEnum::precisionGiven;
    }

    bool widthDynamic() const
    {
        return type & TypeEnum::widthDynamic;
    }

    bool positionalChildren() const
    {
        return type & TypeEnum::positionalChildren;
    }

    bool precisionDynamic() const
    {
        return type & TypeEnum::precisionDynamic;
    }

    bool dynamic() const
    {
        return type & (TypeEnum::widthDynamic | TypeEnum::precisionDynamic);
    }

    bool special() const
    {
        return type & TypeEnum::special;
    }

    void setWidthGiven(bool v = true)
    {
        if (v) {
            type |= TypeEnum::widthGiven;
        } else {
            type &= ~TypeEnum::widthGiven;
        }
    }

    void setPrecisionGiven(bool v = true)
    {
        if (v) {
            type |= TypeEnum::precisionGiven;
        } else {
            type &= ~TypeEnum::precisionGiven;
        }
    }

    void setWidthDynamic(bool v = true)
    {
        if (v) {
            type |= TypeEnum::widthDynamic | TypeEnum::widthGiven;
        } else {
            type &= ~TypeEnum::widthDynamic;
        }
    }

    void setPrecisionDynamic(bool v = true)
    {
        if (v) {
            type |= TypeEnum::precisionDynamic | TypeEnum::precisionGiven;
        } else {
            type &= ~TypeEnum::precisionDynamic;
        }
    }

    void setPositionalChildren(bool v = true)
    {
        if (v) {
            type |= TypeEnum::positionalChildren;
        } else {
            type &= ~TypeEnum::positionalChildren;
        }
    }

    void setSpecial(bool v = true)
    {
        if (v) {
            type |= TypeEnum::special;
        } else {
            type &= ~TypeEnum::special;
        }
    }

    void reset()
    {
        prefix = nullptr;
        suffix = nullptr;
        width = 0;
        precision = 0;
        position = 0;
        widthPosition = 0;
        precisionPosition = 0;
        prefixSize = 0;
        type = 0;
        formatSpecifier = 0;
        fillCharacter = ' ';
    }

    const char* prefix;
    const char* suffix;
    unsigned width;
    unsigned precision;
    unsigned position;
    unsigned widthPosition;
    unsigned precisionPosition;
    unsigned prefixSize;
    unsigned type;
    char formatSpecifier;
    char fillCharacter;

    mutable char buf[32]; // enough for 5 flags, 2 ints, a dot and a specifier.
};

struct alignas(16) FormatNode
{
    FormatNode() = default;

    void reset()
    {
        next = nullptr;
        child = nullptr;
        format = nullptr;
        state.reset();
    }

    FormatNode* next;
    FormatNode* child;
    const char* format;
    FormatState state;
};

struct FormatNodes
{
    static constexpr size_t chunckSize = 64;
    std::array<FormatNode, chunckSize> nodes;
    FormatNodes* next = nullptr;
    size_t index = 0;
};

struct Format
{
    Format() = default;

    Format(const char* f)
        : format(f), wholeFormat(f)
    {
    }

    Format(const Format&) = delete;
    Format& operator=(const Format&) = delete;

    ~Format()
    {
        auto next = chuncks;

        while (next != &nodes) {
            auto tmp = next;

            next = next->next;
            delete tmp;
        }
    }

    FormatNode* getNode();
    FormatNode* buildTree();
    static void printTree(std::ostream& os, FormatNode* node, unsigned indent);
    void dump();

    struct RepeatStackElement
    {
        RepeatStackElement(FormatNode* n, size_t c) : node(n), count(c)
        {
        }

        FormatNode* node;
        size_t count;
    };

    void pushRepeat(FormatNode* node, size_t count)
    {
        if (count != 0) {
            repeatStack.emplace_back(node, count - 1);
        }
    }

    void showErrorContext(FormatNode* node) const;

#if __cplusplus < 201703L
    void errorTail()
    {
    }


    template<typename T, typename... Ts>
    void errorTail(const T& t, const Ts&... ts)
    {
        std::cout << t;
        errorTail(ts...);
    }

    template<typename... Ts>
    void error(const Ts&... ts)
    {
        if (!errorGiven) {
            std::cerr << "TSIO error: ";
            errorTail(ts...);
            std::cerr << ".\n";
            showErrorContext(nextNode);
            std::cerr << std::endl;

            errorGiven = true;
        }
    }

    template<typename... Ts>
    void error(FormatNode* node, const Ts&... ts)
    {
        if (!errorGiven) {
            std::cerr << "TSIO error: ";
            errorTail(ts...);
            std::cerr << ".\n";
            showErrorContext(node);
            std::cerr << std::endl;

            errorGiven = true;
        }
    }

#else
    template<typename... Ts>
    void error(const Ts&... ts)
    {
        if (!errorGiven) {
            std::cerr << "TSIO error: ";
            (std::cerr << ... << ts);
            std::cerr << ".\n";
            showErrorContext(nextNode);
            std::cerr << std::endl;

            errorGiven = true;
        }
    }

    template<typename... Ts>
    void error(FormatNode* node, const Ts&... ts)
    {
        if (!errorGiven) {
            std::cerr << "TSIO error: ";
            (std::cerr << ... << ts);
            std::cerr << ".\n";
            showErrorContext(node);
            std::cerr << std::endl;

            errorGiven = true;
        }
    }
#endif

    bool handleSpecialNodes(FormatNode*& node);
    void getNextNode(bool first = false);
    FormatNode* getNextSibling(FormatNode* node, bool first = false);
    FormatNode* getChild(FormatNode* node);
    void tabTo(unsigned column, bool absolute);

    const char* format = nullptr;
    const char* wholeFormat = nullptr;
    FormatNode* nextNode = nullptr;
    FormatNodes nodes;
    FormatNodes* chuncks = &nodes;
    bool errorGiven = false;
    bool positional = false;
    std::vector<RepeatStackElement> repeatStack;
    std::vector<size_t> indexStack;
    Buffer dest;
};

void outputPointer(Buffer& dest,
                   uintptr_t pNumber,
                   int size,
                   int precision,
                   unsigned type,
                   char fillCharacter);

void printfDetail(Format& format, const std::string& value);
void printfDetail(Format& format, const char* value);
void printfDetail(Format& format, double value);
void printfDetail(Format& format, float value);
void printfDetail(Format& format, bool value);
void printfDetail(Format& format, long long sValue, unsigned long long uValue,
        bool isSigned);

template <typename T, typename std::enable_if<std::is_integral<T>{}, int>::type = 0>
void printfDetail(Format& format, const T& value)
{
    typename std::make_signed<T>::type sValue = value;
    typename std::make_unsigned<T>::type uValue = value;

    printfDetail(format, sValue, uValue, std::is_signed<T>::value);
}

template <typename T>
void printfDetail(Format& format, T* value)
{
    auto& state = format.nextNode->state;
    auto& dest = format.dest;
    char spec = state.formatSpecifier;
    uintptr_t pValue = uintptr_t(value);

    switch (spec) {
        case 'p':
            outputPointer(dest,
                         pValue,
                         state.width,
                         state.precision,
                         state.type,
                         state.fillCharacter);

            break;

        case 'n':
            using baseType = typename std::remove_pointer<T>::type;
            if (!std::is_integral<baseType>{}) {
                format.error("Invalid argumenmt type for %n (expected pointer to integral)");
            }

            *value = baseType(dest.size());

            break;

        default:
            printfDetail(format, static_cast<uintptr_t>(pValue));
    }
}

template <typename T>
void printfDetail(Format& format, const T* value)
{
    auto& state = format.nextNode->state;
    auto& dest = format.dest;
    char spec = state.formatSpecifier;
    uintptr_t pValue = uintptr_t(value);

    switch (spec) {
        case 'p':
            outputPointer(dest,
                         pValue,
                         state.width,
                         state.precision,
                         state.type,
                         state.fillCharacter);

            break;

        default:
            printfDetail(format, static_cast<uintptr_t>(pValue));
    }
}

template <typename T, typename std::enable_if<std::is_class<T>{}, int>::type = 0>
void printfDetail(Format& format, const T& value)
{
    for (const auto& v : value) {
        printfDetail(format, v);
    };
}

template <std::size_t I = 0, typename... Tp>
typename std::enable_if<I == sizeof...(Tp), void>::type
printfNth(Format& format, size_t index, const std::tuple<Tp...>& value)
{
    format.error("Invalid index ", I + 1, " (maximum is ", sizeof...(Tp), ")");
}

template <std::size_t I = 0, typename... Tp>
typename std::enable_if<I < sizeof...(Tp), void>::type
printfNth(Format& format, size_t index, const std::tuple<Tp...>& value)
{
    if (index == I) {
        auto& state = format.nextNode->state;

        if (state.formatSpecifier == '[') {
            containerDetail(format, std::get<I>(value));
        } else if (state.formatSpecifier == '<') {
            tupleDetail(format, std::get<I>(value));
        } else {
            printfDetail(format, std::get<I>(value));
        }
    } else {
        printfNth<I + 1>(format, index, value);
    }
}

template<std::size_t I = 0, typename... Tp>
typename std::enable_if<I == sizeof...(Tp), void>::type
tuplePrintfDetail(Format& format, const std::tuple<Tp...>& value)
{
}

template<std::size_t I = 0, typename... Tp>
typename std::enable_if<I < sizeof...(Tp), void>::type
tuplePrintfDetail(Format& format, const std::tuple<Tp...>& value)
{
    printfDetail(format, std::get<I>(value));
    tuplePrintfDetail<I + 1, Tp...>(format, value);
}

template <typename... Ts>
void printfDetail(Format& format, const std::tuple<Ts...>& value)
{
    tuplePrintfDetail(format, value);
}

template <typename T1, typename T2>
void printfDetail(Format& format, const std::pair<T1, T2>& value)
{
    printfDetail(format, std::tuple<T1, T2>(value));
}

template <typename T, typename std::enable_if<!std::is_integral<T>{}, int>::type = 0>
int toSpec(Format& format, const T& value)
{
    format.error("invalid argument type for '*'");
    return 0;
}

template <typename T, typename std::enable_if<std::is_integral<T>{}, int>::type = 0>
int toSpec(Format& format, const T& value)
{
    return int(value);
}

template <typename T, typename std::enable_if<!std::is_class<T>{} && !std::is_array<T>{}, int>::type = 0>
void containerDetail(Format& format, const T& value)
{
    auto& dest = format.dest;
    auto nextNode = format.nextNode;

    auto child = format.getChild(nextNode);

    format.nextNode = child;
    auto& state = child->state;

    if (state.prefixSize != 0) {
        dest.append(state.prefix, state.prefixSize);
    }

    if (state.formatSpecifier == ']') {
        format.error("Missing format");
    } else if (state.formatSpecifier == '[') {
        containerDetail(format, value);
    } else if (state.formatSpecifier == '<') {
        tupleDetail(format, value);
    } else {
        printfDetail(format, value);
    }

    child = format.getNextSibling(child);

    if (child == nullptr || child->state.formatSpecifier != ']') {
        format.error(child, "Invalid container format (missing %])");
    } else if (!(nextNode->state.type & alternative)) {
        auto& state = child->state;

        if (state.prefixSize != 0) {
            dest.append(state.prefix, state.prefixSize);
        }
    }

    format.nextNode = nextNode;
}

template <typename T, typename std::enable_if<std::is_class<T>{} || std::is_array<T>{}, int>::type = 0>
void containerDetail(Format& format, const T& value)
{
    auto nextNode = format.nextNode;
    auto& dest = format.dest;

    using std::begin;
    using std::end;

    format.indexStack.push_back(0);
    for (auto b = begin(value), e = end(value); b != e;) {
        auto child = format.getChild(nextNode);

        if (child->state.formatSpecifier == ']') {
            format.error(child, "Missing format");
            break;
        }

        format.nextNode = child;
        auto& state = child->state;

        if (state.prefixSize != 0) {
            dest.append(state.prefix, state.prefixSize);
        }

        if (state.formatSpecifier == '[') {
            containerDetail(format, *b);
        } else if (state.formatSpecifier == '<') {
            tupleDetail(format, *b);
        } else {
            printfDetail(format, *b);
        }

        ++b;

        child = format.getNextSibling(child);

        if (child == nullptr || child->state.formatSpecifier != ']') {
            format.error(child, "Invalid container format (expected %])");
        } else if (b != e || !(nextNode->state.type & alternative)) {
            auto& state = child->state;

            if (state.prefixSize != 0) {
                dest.append(state.prefix, state.prefixSize);
            }
        }

        format.indexStack.back()++;
    }

    format.indexStack.pop_back();
    format.nextNode = nextNode;
}

template<std::size_t I = 0, typename... Tp>
typename std::enable_if<I == sizeof...(Tp), void>::type
tupleContainerDetail(Format& format, const std::tuple<Tp...>& value)
{
}

template<std::size_t I = 0, typename... Tp>
typename std::enable_if<I < sizeof...(Tp), void>::type
tupleContainerDetail(Format& format, const std::tuple<Tp...>& value)
{
    auto nextNode = format.nextNode;
    auto& dest = format.dest;

    auto child = format.getChild(nextNode);

    format.nextNode = child;
    auto& state = child->state;

    if (state.prefixSize != 0) {
        dest.append(state.prefix, state.prefixSize);
    }

    if (state.formatSpecifier == '[') {
        containerDetail(format, std::get<I>(value));
    } else if (state.formatSpecifier == '<') {
        tupleDetail(format, std::get<I>(value));
    } else {
        printfDetail(format, std::get<I>(value));
    }

    child = format.getNextSibling(child);

    if (child == nullptr || child->state.formatSpecifier != ']') {
        format.error(child, "Invalid container format(expected %]");
    } else if (I != sizeof...(Tp) - 1 || !(nextNode->state.type & alternative)) {
        auto& state = child->state;

        if (state.prefixSize != 0) {
            dest.append(state.prefix, state.prefixSize);
        }
    }

    format.nextNode = nextNode;
    format.indexStack.back()++;
    tupleContainerDetail<I + 1, Tp...>(format, value);
}

template <typename... Ts>
void containerDetail(Format& format, const std::tuple<Ts...>& value)
{
    format.indexStack.push_back(0);
    tupleContainerDetail(format, value);
    format.indexStack.pop_back();
}

template <typename T1, typename T2>
void containerDetail(Format& format, const std::pair<T1, T2>& value)
{
    format.indexStack.push_back(0);
    tupleContainerDetail(format, std::tuple<T1, T2>(value));
    format.indexStack.pop_back();
}

template<std::size_t I = 0, typename... Tp>
typename std::enable_if<I == sizeof...(Tp), void>::type
tupleTupleDetail(Format& format, const std::tuple<Tp...>& value)
{
    format.error("Only ", I, " formats for tuple elements, ", sizeof...(Tp), " required");
}

template<std::size_t I = 0, typename... Tp>
typename std::enable_if<I < sizeof...(Tp), void>::type
tupleTupleDetail(Format& format, const std::tuple<Tp...>& value)
{
    auto& dest = format.dest;
    auto nextNode = format.nextNode;

    if (nextNode == nullptr) {
        format.error("missing argument in tuple format");
        return;
    }

    auto& state = nextNode->state;

    if (state.prefixSize != 0) {
        dest.append(state.prefix, state.prefixSize);
    }

    if (state.formatSpecifier == '>') {
        format.error("Only ", I, " formats for tuple elements, ", sizeof...(Tp), " required");
    } else if (state.formatSpecifier == '[') {
        containerDetail(format, std::get<I>(value));
    } else if (state.formatSpecifier == '<') {
        tupleDetail(format, std::get<I>(value));
    } else {
        printfDetail(format, std::get<I>(value));
    }

    format.nextNode = format.getNextSibling(format.nextNode);

    format.indexStack.back()++;
    if (I + 1 < sizeof...(Tp)) {
        tupleTupleDetail<I + 1, Tp...>(format, value);
    }
}

template <typename... Ts>
void tupleDetail(Format& format, const std::tuple<Ts...>& value)
{
    auto nextNode = format.nextNode;
    format.indexStack.push_back(0);

    format.nextNode = format.getChild(nextNode);

    if (nextNode->state.positionalChildren()) {
        while (format.nextNode != nullptr) {
            auto nextNode = format.nextNode;
            auto& state = nextNode->state;

            if (state.prefixSize != 0) {
                format.dest.append(state.prefix, state.prefixSize);
            }

            if (state.position == 0) {
                if (state.formatSpecifier == '>') {
                    if (nextNode->next == nullptr) {
                        // nop
                    } else {
                        format.error("Invalid tuple format (expected %>)");
                    }
                } else {
                    format.error("Positional arguments can not be mixed with sequential arguments");
                }
            } else {
                printfNth(format, state.position - 1, value);
            }

            format.nextNode = format.getNextSibling(format.nextNode);
            format.indexStack.back()++;
        }
    } else {
        tupleTupleDetail(format, value);
        auto child = format.getNextSibling(format.nextNode, true);

        if (child == nullptr || child->state.formatSpecifier != '>') {
            format.error("Invalid tuple format (expected %>)");
        } else {
            auto& state = child->state;

            if (state.prefixSize != 0) {
                format.dest.append(state.prefix, state.prefixSize);
            }
        }
    }

    format.indexStack.pop_back();
    format.nextNode = nextNode;
}

template <typename T1, typename T2>
void tupleDetail(Format& format, const std::pair<T1, T2>& value)
{
    auto nextNode = format.nextNode;

    format.nextNode = format.getChild(nextNode);
    format.indexStack.push_back(0);
    tupleTupleDetail(format, std::tuple<T1, T2>(value));
    format.indexStack.pop_back();

    auto child = format.getNextSibling(format.nextNode, true);

    if (child == nullptr || child->state.formatSpecifier != '>') {
        format.error("Invalid tuple format (expected %>)");
    } else {
        auto& state = child->state;

        if (state.prefixSize != 0) {
            format.dest.append(state.prefix, state.prefixSize);
        }
    }

    format.nextNode = nextNode;
}

template <typename T>
void tupleDetail(Format& format, const T& value)
{
    format.error("Invalid argument for tuple format");
}

template <typename T>
void printfOne(Format& format, const T& value)
{
    if (format.nextNode == nullptr && format.repeatStack.empty()) {
        format.error("Extraneous parameter or missing format specifier");
        return;
    }

    auto& state = format.nextNode->state;

    if (state.position != 0 || state.widthPosition != 0 ||
            state.precisionPosition != 0) {
        format.error("Positional arguments can not be mixed with sequential arguments");
        return;
    }

    if (state.dynamic()) {
        if (state.widthDynamic()) {
            int spec = toSpec(format, value);

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

            state.setWidthDynamic(false);
            if (!state.dynamic()) {
                format.getNextNode(true);
            }

            return;
        }

        if (state.precisionDynamic()) {
            int spec = toSpec(format, value);

            if (spec < 0) {
                state.setPrecisionGiven(false);
            } else {
                state.precision = spec;
            }

            state.setPrecisionDynamic(false);
            format.getNextNode(true);
            return;
        }
    }

    if (state.prefixSize != 0) {
        format.dest.append(state.prefix, state.prefixSize);
    }

    if (state.formatSpecifier == '[') {
        containerDetail(format, value);
    } else if (state.formatSpecifier == '<') {
        tupleDetail(format, value);
    } else {
        printfDetail(format, value);
    }

    format.getNextNode();
}

#if __cplusplus < 201703L
inline void printfNth(Format& format, size_t)
{
    format.error("Invalid position in format");
}

template <typename T, typename... Ts>
void printfNth(Format& format, size_t index, const T& value, const Ts&... ts)
{
    auto& state = format.nextNode->state;

    if (index == 1) {
        if (state.formatSpecifier == '[') {
            containerDetail(format, value);
        } else if (state.formatSpecifier == '<') {
            tupleDetail(format, value);
        } else {
            printfDetail(format, value);
        }
    } else {
        printfNth(format, index - 1, ts...);
    }
}

inline void readSpecNum(Format& format, int& dest, size_t)
{
    format.error("Invalid position in format");
    dest = 0;
}

template <typename T, typename... Ts>

void readSpecNum(Format& format, int& dest, size_t index, const T& value, const Ts&... ts)
{
    if (index == 1) {
        dest = toSpec(format, value);
    } else {
        readSpecNum(format, dest, index - 1, ts...);
    }
}
#else
template <typename T, typename... Ts>
bool printfDispatch(Format& format, size_t index, const T& value, const Ts&... ts)
{
    if (index == 0) {
        printfDetail(format, value);
        return true;
    }

    return false;
}

template <typename... Ts>
void printfNth(Format& format, size_t index, const Ts&... ts)
{
    auto i = format.nextNode->state.position;

    if (i > sizeof...(ts)) {
        format.error("Invalid position in format");
        return;
    }

    static_cast<void>(((i--, printfDispatch(format, i, ts)) || ...));
}

template <typename T, typename... Ts>
bool readSpecNumDispatch(Format& format, int& dest, size_t index, const T& value, const Ts&... ts)
{
    if (index == 1) {
        dest = toSpec(format, value);
        return true;
    } else {
        return false;
    }
}

template <typename... Ts>
void readSpecNum(Format& format, int& dest, size_t index, const Ts&... ts)
{
    auto i = index + 1;

    static_cast<void>(((i--, readSpecNumDispatch(format, dest, i, ts)) || ...));
}

#endif

template <typename... Ts>
void printfPositionalOne(Format& format, const Ts&... ts)
{
    auto& state = format.nextNode->state;

    if (state.formatSpecifier == 0) {
        format.dest.append(format.nextNode->state.prefix, format.nextNode->state.prefixSize);
        format.getNextNode();
        return;
    }

    if (state.dynamic()) {
        if (state.widthDynamic()) {
            int spec = 0;

            readSpecNum(format, spec, state.widthPosition, ts...);

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

            state.setWidthDynamic(false);
            if (!state.dynamic()) {
                format.getNextNode(true);
            }

            return;
        }

        if (state.precisionDynamic()) {
            int spec = 0;

            readSpecNum(format, spec, state.precisionPosition, ts...);

            if (spec < 0) {
                state.setPrecisionGiven(false);
            } else {
                state.precision = spec;
            }

            state.setPrecisionDynamic(false);
            format.getNextNode(true);
            return;
        }
    }

    if (state.position == 0) {
        format.error("Positional arguments can not be mixed with sequential arguments");
        format.getNextNode();
        return;
    }

    if (format.nextNode->state.prefixSize != 0) {
        format.dest.append(format.nextNode->state.prefix, format.nextNode->state.prefixSize);
    }

    printfNth(format, state.position, ts...);
    format.getNextNode();
}

#if __cplusplus < 201703L
template <typename T, typename... Ts>
void unpack(Format& format, const T& value, const Ts&... ts)
{
    printfOne(format, value);
    unpack(format, ts...);
}

inline void unpack(Format&)
{
    // nop
}
#endif

template <typename... Ts>
int addSprintf(Format& format, const Ts&... ts)
{
    format.nextNode = &format.nodes.nodes[0];
    format.getNextNode(true);

    if (format.positional) {
        while (format.nextNode != nullptr) {
            printfPositionalOne(format, ts...);
        }
    } else {

#if __cplusplus >= 201703L
        (printfOne(format, ts), ...);
#else
        unpack(format, ts...);
#endif

        if (format.nextNode != nullptr) {
            format.getNextNode(true);
        }

        if (format.nextNode != nullptr) {
            if (format.nextNode->state.formatSpecifier != 0) {
                format.error("Extraneous format or missing parameter");
            }
        }
    }

    return format.errorGiven ? -1 : format.dest.size();
}

template <typename... Ts>
int addSprintf(std::string& dest, const char* f, const Ts&... ts)
{
    Format format(f);

    format.nextNode = format.buildTree();
    int result = addSprintf(format, ts...);

    dest.append(format.dest.data(), format.dest.size());

    return result;
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

class CFormat
{
    public:
        CFormat(const char* f);
        ~CFormat();

        tsioImplementation::Format& getFormat()
        {
            return format;
        }

        void reset()
        {
            format.dest.clear();
            format.errorGiven = false;
        }

    private:
        char* formatCache = nullptr;
        tsioImplementation::Format format;
};

inline std::ostream& operator<<(std::ostream& out, const fmt& f)
{
    return f(out);
}

template <typename... Arguments>
int sprintf(std::string& dest, const char* format, const Arguments&... arguments)
{
    dest.clear();

    return tsioImplementation::addSprintf(dest, format, arguments...);
}

template <typename... Arguments>
int addsprintf(std::string& dest, const char* format, const Arguments&... arguments)
{
    return tsioImplementation::addSprintf(dest, format, arguments...);
}

template <typename... Arguments>
int fprintf(std::ostream& os, const char* format, const Arguments&... arguments)
{
    tsioImplementation::Format fmt(format);

    fmt.nextNode = fmt.buildTree();
    int result = addSprintf(fmt, arguments...);

    os.write(fmt.dest.data(), fmt.dest.size());

    return result;
}

template <typename... Arguments>
int oprintf(const char* format, const Arguments&... arguments)
{
    return fprintf(std::cout, format, arguments...);
}

template <typename... Arguments>
int eprintf(const char* format, const Arguments&... arguments)
{
    return fprintf(std::cerr, format, arguments...);
}

template <typename... Arguments>
std::string fstring(const char* format, const Arguments&... arguments)
{
    std::string result;

    tsioImplementation::addSprintf(result, format, arguments...);

    return result;
}
};

namespace tsioImplementation
{
template <typename... Ts>
int addSprintf(std::string& dest, tsio::CFormat& format, const Ts&... ts)
{
    int result = addSprintf(format.getFormat(), ts...);

    dest.append(format.getFormat().dest.data(), format.getFormat().dest.size());
    return result;
}
};

namespace tsio
{
template <typename... Arguments>
int sprintf(std::string& dest, CFormat& format, const Arguments&... arguments)
{
    dest.clear();

    format.reset();
    return tsioImplementation::addSprintf(dest, format, arguments...);
}

template <typename... Arguments>
int addsprintf(std::string& dest, CFormat& format, const Arguments&... arguments)
{
    format.reset();
    return tsioImplementation::addSprintf(dest, format, arguments...);
}

template <typename... Arguments>
int fprintf(std::ostream& os, CFormat& format, const Arguments&... arguments)
{
    format.reset();
    int result = tsioImplementation::addSprintf(format.getFormat(), arguments...);

    os.write(format.getFormat().dest.data(), format.getFormat().dest.size());

    return result;
}

template <typename... Arguments>
int oprintf(CFormat& format, const Arguments&... arguments)
{
    return fprintf(std::cout, format, arguments...);
}

template <typename... Arguments>
int eprintf(CFormat& format, const Arguments&... arguments)
{
    return fprintf(std::cerr, format, arguments...);
}

template <typename... Arguments>
std::string fstring(CFormat& format, const Arguments&... arguments)
{
    std::string result;

    format.reset();
    tsioImplementation::addSprintf(result, format, arguments...);

    return result;
}
};

#endif
