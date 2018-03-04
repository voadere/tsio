#include "tsio.h"
#include <map>
#include <vector>

/*
 * This example programs shows different ways to format the contents
 * of a map.
 *
 * For more information on the tsio::oprint formats, refer to the documentation.
 */

using tsio::oprintf;

std::map<int, std::vector<std::string>> m = {
    {1, {"one", "un", "uno"}},
    {2, {"two", "deux", "dos"}},
    {3, {"three", "trois", "tres", "drei"}}
};

/*
 * The dump function uses the simplest format thinkable to dump the content
 * of a map.
 *
 * It uses the fact that, when a container (map, vector, tuple etc...)
 * is given as a parmeter to a simple format, this format will be used
 * recursively for all elements of the container.
 */

static void dump()
{
    oprintf("%6s\n", m);
}

/*
 * the print function uses a container format to add decorations ( {} ) around
 * each map element.
 */

static void print()
{
    oprintf("%[{ %6s }\n%]", m);
}

/*
 * The show function uses a tuple format to allow different formats for
 * the key and for the value of each element
 */

static void show()
{
    oprintf("%[%<{ %2d: %-6s }%>\n%]", m);
}

/*
 * The shine function uses separate formats for all elements of map, tuple
 * and vector elemnts.
 */

static void shine()
{
    oprintf("{\n%[   %<{ %2d: %#[%-s, %] }%>\n%]}\n", m);
}

static void  overview()
{
    oprintf("{\n"               // starting brace
            "%[   "             // start container format for map
              "%<"              // start tuple format for map element
              "{ "              // generate open brace
                "%2d: "         // format for element key
                "%#["           // start container format for element value
                  "%-s, "       // format for element of element and , separator
                "%] }"          // end container format for element value
              "%>\n"            // end tuple format for map element
            "%]}\n",            // end container format for map
            m);                 
}

int main()
{
    oprintf("\ndump:\n");
    dump();

    oprintf("\n\nprint:\n");
    print();

    oprintf("\n\nshow:\n");
    show();

    oprintf("\n\nshine:\n");
    shine();

    oprintf("\n\n");

    if (false) {
        overview();             // suppress compiler warning
    }
}

