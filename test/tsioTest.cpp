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
#include <map>
#include <set>

using namespace tsio;

static size_t count = 0;

static const char* flagsCombo[] = {"",     " ",   " 0", " 0-", " 0-+", " 0-+#", " 0-#", " 0+",
                                   " 0+#", " 0#", " -", " -+", " -+#", " -#",   " +",   " +#",
                                   " #",   "0",   "0-", "0-+", "0-+#", "0-#",   "0+",   "0+#",
                                   "0#",   "-",   "-+", "-+#", "-#",   "+",     "+#",   "#"};

template <typename E, typename F>
void expect(const E& expected, const F& found)
{
    if (expected != found) {
        std::cerr << "Expected '" << expected << "'; found '" << found << "'." << std::endl;
    }

    count++;
}

template <typename E, typename F, typename T>
void expect(const E& expected, const F& found, const T& value)
{
    if (expected != found) {
        std::cerr << "Expected '" << expected << "'; found '" << found << "' for value '" << value << "'."
                  << std::endl;
    }

    count++;
}

template <typename T>
void test(const T& value,
          const char* specs,
          int specNum1,
          int specNum2,
          int* dyn1 = nullptr,
          int* dyn2 = nullptr,
          const char* modifier = "")
{
    char buf[128];
    std::string text;
    std::string f;

    for (const char* pt = specs; *pt != 0; ++pt) {
        char spec = *pt;

        for (const auto& flag : flagsCombo) {
            f = "%";
            f += flag;

            if (dyn1 != nullptr) {
                f += '*';
            } else if (specNum1 >= 0) {
                f += std::to_string(specNum1);
            }

            if (dyn2 != nullptr) {
                f += ".*";
            } else if (specNum2 < 0) {
                if (specNum2 == -2) {
                    f += '.';
                }
            } else {
                f += '.';
                f += std::to_string(specNum2);
            }

            f += modifier;
            f += spec;

            std::string fmt('%' + f + "   \t\"" + f + "\".");
            const char* format = fmt.c_str();

            if (dyn1 != nullptr) {
                if (dyn2 != nullptr) {
                    sprintf(buf, format, *dyn1, *dyn2, value);
                    sprintf(text, format, *dyn1, *dyn2, value);
                } else {
                    sprintf(buf, format, *dyn1, value);
                    sprintf(text, format, *dyn1, value);
                }
            } else if (dyn2 != nullptr) {
                sprintf(buf, format, *dyn2, value);
                sprintf(text, format, *dyn2, value);
            } else {
                sprintf(buf, format, value);
                sprintf(text, format, value);
            }

            //          std::cout << buf << std::endl;

            expect(buf, text, value);
            count++;
        }
    }
}

static void testStringIntegral()
{
    int intValues[] = {1234, -1234, 1, -1, 0, int(0x80000000)};
    int spec1s[] = {-1, 3, 15, 0};
    int spec2s[] = {-1, -2, 3, 0};

    for (auto spec1 : spec1s) {
        for (auto spec2 : spec2s) {
            for (auto value : intValues) {
                test(value, "diouxX", spec1, spec2);
            }

            test(0x8000000000000000ll, "diouxX", spec1, spec2, nullptr, nullptr, "ll");
            test('c', "cdiouxX", spec1, spec2);
            test(true, "diouxX", spec1, spec2);
            test(false, "diouxX", spec1, spec2);
        }
    }

    unsigned untValues[] = {1234, 1, 0, 0x80000000};

    for (auto spec1 : spec1s) {
        for (auto spec2 : spec2s) {
            for (auto value : untValues) {
                test(value, "diouxX", spec1, spec2);
            }

            test(0x8000000000000000ull, "diouxX", spec1, spec2, nullptr, nullptr, "ll");
        }
    }

}

static void testStringFloatingPoint()
{
    double floatValues[] = {12.34, 987.0, 56e35, 0., 456.78, -456.78};
    int spec1s[] = {15, 0, -1};
    int spec2s[] = {3, 0, -1, -2};

    for (auto spec1 : spec1s) {
        for (auto spec2 : spec2s) {
            for (auto value : floatValues) {
                test(value, "aAfFeEgG", spec1, spec2);
            }
        }
    }
}

static void testStringPointer()
{
    const char* t1 = "Another test";

    int spec1s[] = {15, 0, -1};
    int spec2s[] = {3, 0, -1, -2};

    for (auto spec1 : spec1s) {
        for (auto spec2 : spec2s) {

            test("The test", "s", spec1, spec2);
            test(t1, "diouxXs", spec1, spec2);
        }
    }
}

static void testStringDynamics()
{
    int intValues[] = {1234, -1234, 1, -1, 0, int(0x80000000)};
    double floatValues[] = {1234.56, -1234.56, -1., 0.};
    int spec1s[] = {15, -15, 0};
    int spec2s[] = {3, 10, -1, 0};

    for (auto spec1 : spec1s) {
        for (auto spec2 : spec2s) {
            for (auto value : intValues) {
                test(value, "diouxX", -1, spec2, &spec1, nullptr);
                test(value, "diouxX", spec1, -1, nullptr, &spec2);
                test(value, "diouxX", -1, -1, &spec1, &spec2);
            }

            for (auto value : floatValues) {
                test(value, "aAfFeEgG", -1, spec2, &spec1, nullptr);
                test(value, "aAfFeEgG", spec1, -1, nullptr, &spec2);
                test(value, "aAfFeEgG", -1, -1, &spec1, &spec2);
            }
        }
    }
}

static void testPrintfN()
{
    std::string text;
    int iSize;
    int jSize;
    short sSize;
    char buf[30];

    sprintf(buf, "12345%n67890", &iSize);
    sprintf(text, "12345%n67890", &jSize);
    expect("1234567890", text);
    expect(iSize, jSize);

    sprintf(text, "12345678%n90", &sSize);
    expect("1234567890", text);
    expect(8, sSize);

    sprintf(text, "Hello ");
    addsprintf(text, "World%n!", &sSize);
    expect("Hello World!", text);
    expect(5, sSize);
}

static void testPrintString()
{
    std::string text;
    std::string s = "abcde";

    sprintf(text, "%s", s);
    expect("abcde", text);
}

static void testPositional()
{
    std::string text;
    char buf[128];

    sprintf(text, "%2$s, %1$s!", "world", "Hello");
    sprintf(buf, "%2$s, %1$s!", "world", "Hello");
    expect(buf, text);

    sprintf(text, "%2$-*4$s, %1$.*3$s!", "world", "Hello", 3, 20);
    sprintf(buf, "%2$-*4$s, %1$.*3$s!", "world", "Hello", 3, 20);
    expect(buf, text);
}

static void testRepeatingFormats()
{
    std::string text = fstring("***%1{-%}***");

    expect("***-***", text);

    text = fstring("***%10{-%}***");
    expect("***----------***", text);

    text = fstring("***%3{%d%}***", 1, 2, 3);
    expect("***123***", text);

    text = fstring("***%3{%d%2{  %}%}***", 1, 2, 3);
    expect("***1    2    3    ***", text);

    text = fstring("%10{-=%}");
    expect("-=-=-=-=-=-=-=-=-=-=", text);

    text = fstring("%5{--%3{**%}==%}");
    expect("--******==--******==--******==--******==--******==", text);

    text = fstring("%*{abc%}", 5);
    expect("abcabcabcabcabc", text);

    text = fstring("%3{%4d%}", 1, 22, 333);
    expect("   1  22 333", text);

    text = fstring("%3{%4{*%}%}");
    expect("************", text);

    text = fstring("%5d %2{**%} %s %*{+%} %*d %*{ %d%} %2{!%}?", 1, "two", 3, 4, 3, 5, 6, 7, 8, 9, 10);
    expect("    1 **** two +++    3  6 7 8 9 10 !!?", text);

    text = fstring("prefix %4{*%} %3{%3$4d%} %*4${!%}", 1, 22, 333, 2);
    expect("prefix ****  333 333 333 !!", text);
}

static void testBinaryFormat() {
    std::string text = fstring("%b %#b %B %#B", 2, 1234, 99, 9876);
    expect("10 0b10011010010 1100011 0B10011010010100", text);
}

static void extensions()
{
    std::string text = fstring("%C %C", '\x12', 'a');

    expect(". a", text);

    text = fstring("%#C %#C %#C", '\a', 'a', '\x5');
    expect("\\a a \\005", text);

    std::array<char, 256> a1;

    for (int i = 0; i < 256; ++i) {
        a1[i] = char(i);
    }

    text = fstring("%#5C", a1);

    expect(" \\000 \\001 \\002 \\003 \\004 \\005 \\006   \\a   \\b   "
           "\\t   \\n   \\v   \\f   \\r \\016 \\017 \\020 \\021 \\022 "
           "\\023 \\024 \\025 \\026 \\027 \\030 \\031 \\032 \\033 \\034 "
           "\\035 \\036 \\037         !   \\\"    #    $    %    &   \\'    "
           "(    )    *    +    ,    -    .    /    0    1    "
           "2    3    4    5    6    7    8    9    :    ;    "
           "<    =    >    ?    @    A    B    C    D    E    "
           "F    G    H    I    J    K    L    M    N    O    "
           "P    Q    R    S    T    U    V    W    X    Y    "
           "Z    [   \\\\    ]    ^    _    `    a    b    c    "
           "d    e    f    g    h    i    j    k    l    m    "
           "n    o    p    q    r    s    t    u    v    w    "
           "x    y    z    {    |    }    ~ \\177 \\200 \\201 "
           "\\202 \\203 \\204 \\205 \\206 \\207 \\210 \\211 \\212 \\213 "
           "\\214 \\215 \\216 \\217 \\220 \\221 \\222 \\223 \\224 \\225 "
           "\\226 \\227 \\230 \\231 \\232 \\233 \\234 \\235 \\236 \\237 "
           "\\240 \\241 \\242 \\243 \\244 \\245 \\246 \\247 \\250 \\251 "
           "\\252 \\253 \\254 \\255 \\256 \\257 \\260 \\261 \\262 \\263 "
           "\\264 \\265 \\266 \\267 \\270 \\271 \\272 \\273 \\274 \\275 "
           "\\276 \\277 \\300 \\301 \\302 \\303 \\304 \\305 \\306 \\307 "
           "\\310 \\311 \\312 \\313 \\314 \\315 \\316 \\317 \\320 \\321 "
           "\\322 \\323 \\324 \\325 \\326 \\327 \\330 \\331 \\332 \\333 "
           "\\334 \\335 \\336 \\337 \\340 \\341 \\342 \\343 \\344 \\345 "
           "\\346 \\347 \\350 \\351 \\352 \\353 \\354 \\355 \\356 \\357 "
           "\\360 \\361 \\362 \\363 \\364 \\365 \\366 \\367 \\370 \\371 "
           "\\372 \\373 \\374 \\375 \\376 \\377",
           text);

    text = fstring("%S", "12\a\b\f34");
    expect("12...34", text);


    text = fstring("%#S", "12\a\b\f34");
    expect("12\\a\\b\\f34", text);

    text = fstring("%^20s", "1234");
    expect("        1234        ", text);

    text = fstring("%^20d", 1234);
    expect("        1234        ", text);

    text = fstring("%^20s", "abc");
    expect("        abc         ", text);

    text = fstring("%+'*\"@20d", 123);
    expect("+****************123", text);

    text = fstring("%+\"@20d", 123);
    expect("@@@@@@@@@@@@@@@@+123", text);

    text = fstring("%^\"*20s", "abc");
    expect("********abc*********", text);

    std::string str("This is a std::string");

    text = fstring("%s %s %s %s %s %s %s", 123, 234.567, true, false, str, 0xffffffffu, int(0xffffffff));
    expect("123 234.567 true false This is a std::string 4294967295 -1", text);

    text = fstring("%06s %5.2s %10.3s", 123, 234.567, 98.765);
    expect("000123 2.3e+02       98.8", text);

    text = fstring("%'*7d', %#'*7x", -23, 0xab);
    expect("-****23', 0x***ab", text);

    text = fstring("%\"*7d', %#\"*7x", -23, 0xab);
    expect("****-23', ***0xab", text);

    text = fstring("%\"*20s", "abc");
    expect("*****************abc", text);

    std::vector<int> v = {10, 200, 3000};

    text = fstring("vector { %10d }", v);
    expect("vector {         10       200      3000 }", text);

    std::array<double, 3> a = {10.11, 200.222, 3000.3333};

    text = fstring("array { %10.3f }", a);
    expect("array {     10.110   200.222  3000.333 }", text);

    std::set<int> s = {1, 100, 1000, 2, 3};

    text = fstring("set {%5d }", s);
    expect("set {    1    2    3  100 1000 }", text);

    text = fstring("%*4${!%}", 1, 22, 333, 2);
    expect("!!", text);

    text = fstring("%[v=%d, %]", 66);
    expect("v=66, ", text);

    text = fstring("%#[v=%d, %]", 66);
    expect("v=66", text);

    std::vector<int> v1 = {9, 8, 7, 6};
    double fa[] = {1.2, 2.3, 3.4, 4.55555};

    text = fstring("%[v=%d, %]", v1);
    expect("v=9, v=8, v=7, v=6, ", text);

    text = fstring("{ %#[v=%d, %] }", v1);
    expect("{ v=9, v=8, v=7, v=6 }", text);

    text = fstring("{ %#[v=%.2f, %] }", fa);
    expect("{ v=1.20, v=2.30, v=3.40, v=4.56 }", text);

    auto t = std::make_tuple(1, 2.3, "four");

    text = fstring("%5s", t);
    expect("    1  2.3 four", text);

    text = fstring("%[v=%s, %]", t);
    expect("v=1, v=2.3, v=four, ", text);

    text = fstring("{ %#[v=%s, %] }", t);
    expect("{ v=1, v=2.3, v=four }", text);

    text = fstring("%<%5d %5.2f %10s%>", t);
    expect("    1  2.30       four", text);

    std::map<int, const char*> m = { {1, "one"}, {3, "three"}, {2, "two"} };

    text = fstring("%#[%<{ key: %3d, value: %5s }%>, %]", m);
    expect("{ key:   1, value:   one }, { key:   2, value:   two }, { key:   3, value: three }", text);

    text = fstring("%#{ { %[%6s%] }", m);
    expect(" {      1   one     2   two     3 three }", text);

    text = fstring("%5s", std::make_pair(2, 4));
    expect("    2    4", text);

    text = fstring("%d%#10T%d%#30T%d%#15T%d", 1, 2222, 3, 4);
    expect("1        2222                3\n              4", text);

    text = fstring("%d%5T%d%5T%d%5T%d%5T%d", 1, 1234, 123456, 12345, 9);
    expect("1    1234 123456    12345     9", text);
}

static void run()
{
    testStringIntegral();
    testStringFloatingPoint();
    testStringPointer();
    testStringDynamics();
    testPrintfN();
    testPrintString();
    testRepeatingFormats();
    testBinaryFormat();
    testPositional();
    extensions();
}

static void test()
{
    std::string text;

    std::cout << text;
}

static void examples()
{
    std::string text;

    std::cout << text << '\n';
}

int main(int argc, char* argv[])
{
    test();

    long repeatCount = 1;

    if (argc > 1) {
        repeatCount = atol(argv[1]);
    }

    for (int i = 0; i < repeatCount; ++i) {
        run();
    }

    examples();

    oprintf("    %d sprintf tests executed in %4.2f seconds\n",
            count,
            double(clock() / double(CLOCKS_PER_SEC)));
}
