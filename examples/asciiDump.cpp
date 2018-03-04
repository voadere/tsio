#include <tsio.h>

#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

/*
 * This example generates a dump of a file.
 * If the program is called with the -x flag then it produces output like:

   00057e20 203 00 00 00 fc 6f 05 08 fc df 00 00 04 00 00 00            .....o..........

 * else if the program is called with the -C flag it produces

   00057e20  03 00 00 00 fc 6f 05 08  fc df 00 00 04 00 00 00  |.....o..........|

 * else it produces

   00057e20   \003 \000 \000 \000 \374    o \005   \b         .....o..

 * each output line is produced with a single format.  To better understand
 * how these formats work, they are annotated in the function 'overview'.
 *
 * For more information on the tsio::oprint formats, refer to the documentation.
 */

enum {
    asciiFormat,
    hexFormat,
    classicFormat
};

static void usage(const char* name)
{
    std::cerr << "Usage: " << name << " [-x | -C] <file name>\n";
}

/*
 * This example s based on the feature that a std::string can be formatted
 * as a string or as a container of characters.
 */

static void dump(const char* fileName, unsigned format)
{
    using namespace tsio;

    std::ifstream is(fileName);

    if (!is) {
        std::cerr << "Could not open file '" << fileName << "'\n";
        exit(1);
    }

    size_t totalCount = 0;

    for (;;) {
        char buf[16];
        is.read(buf, sizeof(buf));

        size_t count = is.gcount();

        if (count == 0) {
            break;
        }

        std::string s(buf, count);

        switch (format) {
            case asciiFormat:
                oprintf("%2{%08x  %[%#5C%]%#60T%S\n%}",
                        totalCount, s.substr(0,8), s.substr(0,8),
                        totalCount + 8, s.substr(8, 8), s.substr(8, 8));

                break;

            case hexFormat:
                oprintf("%08x  %[%02x %]%#70T%S\n", totalCount, s, s);
                break;

            case classicFormat:
                oprintf("%08x %2{ %[%02x %]%}%#61T|%S|\n", totalCount,
                        s.substr(0,8), s.substr(8, 8), s);
                break;

        }

        totalCount += count;
    }

    oprintf("\n");
}

/*
 * The function overview is equal to the function dump, but the formats
 * are annotated.
 */

static void overview(const char* fileName, unsigned format)
{
    using namespace tsio;

    std::ifstream is(fileName);

    if (!is) {
        std::cerr << "Could not open file '" << fileName << "'\n";
        exit(1);
    }

    size_t totalCount = 0;

    for (;;) {
        char buf[16];
        is.read(buf, sizeof(buf));

        size_t count = is.gcount();

        if (count == 0) {
            break;
        }

        std::string s(buf, count);

        switch (format) {
            case asciiFormat:
                oprintf("%2{"           // repeat twice
                          "%08x  "      // print totalCount
                          "%["          // begin container format
                            "%#5C"      // print one character or escape sequence
                          "%]"          // end container format
                          "%#60T"       // skip to column 60
                          "%S"          // print string; replace unprintable width '.'
                          "\n"          // new linme
                        "%}",           // end repeat
                        totalCount, s.substr(0,8), s.substr(0,8),
                        totalCount + 8, s.substr(8, 8), s.substr(8, 8));

                break;

            case hexFormat:
                oprintf("%08x  "        // print totalCount
                        "%["            // begin container format
                          "%02x "       // print a character in hex
                        "%]"            // end container format
                        "%#70T"         // skip to column 70
                        "%S"            // print string; replace unprintable width '.'
                        "\n",           // new linme
                        totalCount, s, s);
                break;

            case classicFormat:
                oprintf("%08x "
                        "%2{ "          // repeat twice
                          "%["          // begin container forma
                            "%02x "     // print a character in hex
                          "%]"          // end container format
                        "%}"            // end repeat
                        "%#61T"         // skip to column 61
                        "|"             // verical bar
                        "%S"            // print string; replace unprintable width '.'
                        "|\n",          // verical bar and new line
                        totalCount,
                        s.substr(0,8), s.substr(8, 8), s);
                break;

        }

        totalCount += count;
    }

    oprintf("\n");
}

int main(int argc, char* argv[])
{
    unsigned format = asciiFormat;
    int opt;

    while ((opt = getopt(argc, argv, "xC")) != -1) {
        switch (opt) {
            case 'x':
                format = hexFormat;
                break;

            case 'C':
                format = classicFormat;
                break;

            default:
                usage(argv[0]);
                exit(1);
        }
    }

    if (optind >= argc) {
        usage(argv[0]);
        exit(1);
    }

    const char* fileName = argv[optind];

    dump(fileName, format);

    if (false) {
        overview(fileName, format);     // suppress compiler warning
    }
}
