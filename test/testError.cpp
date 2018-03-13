#include "tsio.h"

using namespace tsio;

static void test()
{
    std::string dest;
    int i = 1234;
    double d = 123.45;
    std::vector<int> v = { 1, 2, 3 };
    std::tuple<int, int, int> t = { 9, 8, 7};
    auto p = std::make_pair(1, "one");

    // general
    dest = fstring("  1: %<%s%>", 1);
    dest = fstring("  2: %n.", &d);
    dest = fstring("  3: %*d.", d, i);
    dest = fstring("  4: %d %d", 1, 2, 3);
    dest = fstring("  5: %d %d", 1);
    dest = fstring("  6: %5d:", d);
    dest = fstring("  7: %5f:", i);

    // loop formats
    dest = fstring("101: %[ %d %}", v);
    dest = fstring("102: %[ %d", v);
    dest = fstring("103: %[ %d %d %]", v);
    dest = fstring("104: %[ %]", v);
    dest = fstring("105: %[ %2{%d%}", v);
    dest = fstring("106: %[ %d, %]", 66);

    // sequence formats
    dest = fstring("201: %< %d %}", t);
    dest = fstring("202: %< %d", t);
    dest = fstring("203: %< %d %}", p);
    dest = fstring("204: %< %d", p);
    dest = fstring("205: %< %5d%5d %>", t);
    dest = fstring("206: %< %5d%5d%5d%5d %>", t);
    dest = fstring("207: %< %2{%5d%} %>", t);
    dest = fstring("208: %< %4{%5d%} %>", t);
    dest = fstring("209: %< %>", t);
    dest = fstring("210: %< %3$d, %4$d%>", t);

    // positional
    dest = fstring("301: %d %1$d", 1);
    dest = fstring("302: %2$d %1$d", 1);

    // dynamic
    dest = fstring("401: %*d", 1);
    dest = fstring("402: %[%*d%]", v, 5);

//  std::cout << dest << std::endl;
}

int main()
{
    test();
}

