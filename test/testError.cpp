#include "tsio.h"

using namespace tsio;

static void test()
{
    std::string dest;
    int i = 1234;
    double d = 123.45;
    std::vector<int> v = { 1, 2, 3 };
    std::tuple<int, int>t = { 1, 2};
    auto p = std::make_pair(1, "one");

    dest = fstring("%<%s%>", 1);
    dest = fstring("value: %n.", &d);
    dest = fstring("value: %*d.", d, i);
    dest = fstring("vector: %[ %d %}", v);
    dest = fstring("vector: %[ %d", v);
    dest = fstring("vector: %< %d %}", t);
    dest = fstring("vector: %< %d", t);
    dest = fstring("vector: %< %d %}", p);
    dest = fstring("vector: %< %d", p);

    dest = fstring("%d %d", 1, 2, 3);
    dest = fstring("%d %d", 1);
    dest = fstring("%d %1$d", 1);
    dest = fstring("%2$d %1$d", 1);
}

int main()
{
    test();
}

