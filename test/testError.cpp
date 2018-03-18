#include "tsio.h"
#include <cassert>

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
    assert(sprintf(dest,"  1: %<%s%>", 1) < 0);
    assert(sprintf(dest,"  2: %n.", &d) < 0);
    assert(sprintf(dest,"  3: %n.", i) < 0);
    assert(sprintf(dest,"  4: %*d.", d, i) < 0);
    assert(sprintf(dest,"  5: %*d.", &i, i) < 0);
    assert(sprintf(dest,"  6: %d %d", 1, 2, 3) < 0);
    assert(sprintf(dest,"  7: %d %d", 1) < 0);
    assert(sprintf(dest,"  8: %5d:", d) < 0);
    assert(sprintf(dest,"  9: %5f:", i) < 0);

    // loop formats
    assert(sprintf(dest,"101: %[ %d %}", v) < 0);
    assert(sprintf(dest,"102: %[ %d", v) < 0);
    assert(sprintf(dest,"103: %[ %d %d %]", v) < 0);
    assert(sprintf(dest,"104: %[ %]", v) < 0);
    assert(sprintf(dest,"105: %[ %2{%d%}", v) < 0);
    assert(sprintf(dest,"106: %[ %d, %]", 66) < 0);
    assert(sprintf(dest,"107: %[", v) < 0);
    assert(sprintf(dest,"108: %]", v) < 0);

    // sequence formats
    assert(sprintf(dest,"201: %< %d %}", t) < 0);
    assert(sprintf(dest,"202: %< %d", t) < 0);
    assert(sprintf(dest,"203: %< %d %}", p) < 0);
    assert(sprintf(dest,"204: %< %d", p) < 0);
    assert(sprintf(dest,"205: %< %5d%5d %>", t) < 0);
    assert(sprintf(dest,"206: %< %5d%5d%5d%5d %>", t) < 0);
    assert(sprintf(dest,"207: %< %2{%5d%} %>", t) < 0);
    assert(sprintf(dest,"208: %< %4{%5d%} %>", t) < 0);
    assert(sprintf(dest,"209: %< %>", t) < 0);
    assert(sprintf(dest,"210: %< %3$d, %4$d%>", t) < 0);
    assert(sprintf(dest,"211: %<", p) < 0);
    assert(sprintf(dest,"212: %>", p) < 0);

    // positional
    assert(sprintf(dest,"301: %d %1$d", 1, 2) < 0);
    assert(sprintf(dest,"302: %1$d %d", 1, 2) < 0);
    assert(sprintf(dest,"303: %2$d %1$d", 1) < 0);
    assert(sprintf(dest,"304: %1$", 1) < 0);
    assert(sprintf(dest,"305: %2$*.*d", 1, 2, 3) < 0);
    assert(sprintf(dest,"306: %2$*1$.*d", 1, 2, 3) < 0);

    // dynamic
    assert(sprintf(dest,"401: %*d", 1) < 0);
    assert(sprintf(dest,"402: %[%*d%]", v, 5) < 0);

    // repeat
    assert(sprintf(dest,"501: %{") < 0);
    assert(sprintf(dest,"502: %}") < 0);
//  std::cout << dest << std::endl;
}

int main()
{
    test();
}

