#include "tsio.h"

/**********************************************************************************
 * This example program generates a printed report for quarterly customer data.
 *
 * The report data are organized in a vector:
 *     using ReportData = std::vector<CustomerData>;
 *
 * The customer data are organized as a tuple:
 *     using CustomerData = std::tuple<unsigned, std::string, double, MonthlyData>;
 *
 * The fields of this tuple the customer number, name, quarterly total and
 * monthly percentages respectively.
 *
 * The MontlyData are organized as an std::array:
 *     using MonthlyData = std::array<float, 3>;
 *
 * The report that will be generated looks like this:
 *
 * Yearly report for 2018Q1
 * ------------------------------------------------------------------------
 *     1 Jetson                  123.45       1.20%  2.30%  3.40%
 *     3 FlintStone              843.99       2.86%  3.96%  5.06%
 *    17 Rubble                 1034.60       3.52%  4.87%  6.22%
 * ------------------------------------------------------------------------
 *
 * The setUp function creates some report data.
 * 
 * All other functions create the same report, and show how the ostream
 * based solution can be changed into a, arguably much simpler,
 * printf based solution.
 *
 * The proper functioning of ever function can be verified by running the program.
 *
 * For more information on the tsio::oprint formats, refer to the documentation.
 */


// define the data types used in the customer report
using MonthlyData = std::array<float, 3>;
using CustomerData = std::tuple<unsigned, std::string, double, MonthlyData>;
using ReportData = std::vector<CustomerData>;

/*
 * The setUp function creates some data for the customer report
 */

static ReportData setUp()
{
    ReportData report;
    MonthlyData monthly = { 1.2f, 2.3f, 3.4f };

    report.push_back(make_tuple(1, "Jetson", 123.45, monthly));

    for (auto& m : monthly) {
        m += 1.66f;
    }

    report.push_back(make_tuple(3, "FlintStone", 843.99, monthly));

    for (auto& m : monthly) {
        m *= 1.23f;
    }

    report.push_back(make_tuple(17, "Rubble", 1034.6,  monthly));

    return report;
}

/*
 * The function withOstream creates the customer report using only std::ostream
 * formatting.
 *
 * It is used as the basis for the following functions.
 */

static void withOstream(const ReportData& report, unsigned year, unsigned quarter)
{
    std::cout << "Yearly report for " << std::setw(4) << year << "Q" << quarter << '\n' <<
        std::setw(72) << std::setfill('-') << "" << std::setfill(' ') << '\n';

    unsigned lineNumber = 0;

    for (const auto& customer : report) {
        lineNumber++;

        std::cout << std::setw(3) << lineNumber << " " <<
            std::setw(5) << std::get<0>(customer) << " " <<
            std::setw(20) << std::left << std::get<1>(customer) << std::right <<
            std::setw(10) << std::setprecision(2) << std::fixed << std::get<2>(customer) <<
            "     ";

        for (const auto& monthly : std::get<3>(customer)) {
            std::cout << std::setw(6) << std::setprecision(2) << monthly << '%';
        }

        std::cout << '\n';
    }

    std::cout << std::setw(72) << std::setfill('-') << "" << std::setfill(' ') << '\n';
}

using tsio::oprintf;

/*
 * The following functions named stepNN are progressive steps in morphing
 * the ostream based solution into a tsio::oprintf based solution.
 *
 * The steps are small and easy to follow.
 *
 * The tsio::oprintf function is equivalent to the std::printf function,
 * but it writes to std::cout.
 */

/*
 * The function step1 replaces the line
 *
 *     std::cout << std::setw(6) << std::setprecision(2) << monthly << '%';
 *
 * with a oprintf statement.
 */

static void step1(const ReportData& report, unsigned year, unsigned quarter)
{
    std::cout << "Yearly report for " << std::setw(4) << year << "Q" << quarter << '\n' <<
        std::setw(72) << std::setfill('-') << "" << std::setfill(' ') << '\n';

    unsigned lineNumber = 0;

    for (const auto& customer : report) {
        lineNumber++;

        std::cout << std::setw(3) << lineNumber << " " <<
            std::setw(5) << std::get<0>(customer) << " " <<
            std::setw(20) << std::left << std::get<1>(customer) << std::right <<
            std::setw(10) << std::setprecision(2) << std::fixed << std::get<2>(customer) <<
            "     ";

        for (const auto& monthly : std::get<3>(customer)) {
            oprintf("%6.2f%%", monthly);
            //       ^^^^^   print a floating point format
            //            ^^ print a single %
        }

        std::cout << '\n';
    }

    std::cout << std::setw(72) << std::setfill('-') << "" << std::setfill(' ') << '\n';
}

/*
 * The function step2 replaces the formatting of the monthly array
 *
 *      for (const auto& monthly : std::get<3>(customer)) {
 *          oprintf("%6.2f%%", monthly);
 *      }
 *
 *      std::cout << '\n';
 *
 * with a oprintf statement that uses a container format.
 *
 * The container format is defined as '%[ fff %]' and repeats the format fff
 * for each element of the array.
 */

static void step2(const ReportData& report, unsigned year, unsigned quarter)
{
    std::cout << "Yearly report for " << std::setw(4) << year << "Q" << quarter << '\n' <<
        std::setw(72) << std::setfill('-') << "" << std::setfill(' ') << '\n';

    unsigned lineNumber = 0;

    for (const auto& customer : report) {
        lineNumber++;

        std::cout << std::setw(3) << lineNumber << " " <<
            std::setw(5) << std::get<0>(customer) << " " <<
            std::setw(20) << std::left << std::get<1>(customer) << std::right <<
            std::setw(10) << std::setprecision(2) << std::fixed << std::get<2>(customer) <<
            "     ";

        oprintf("%[%6.2f%%%]\n", std::get<3>(customer));
        //       ^^          start container format
        //         ^^^^^^^   format to be used for each element
        //                ^^ end container format
    }

    std::cout << std::setw(72) << std::setfill('-') << "" << std::setfill(' ') << '\n';
}

/*
 * The function step3 replaces the whole inner loop body
 *
 *      std::cout << std::setw(5) << std::get<0>(customer) << " " <<
 *          std::setw(20) << std::left << std::get<1>(customer) << std::right <<
 *          std::setw(10) << std::setprecision(2) << std::fixed << std::get<2>(customer) <<
 *          "     ";
 *
 *      oprintf("%[%6.2f%%%]\n", std::get<3>(customer));
 *
 * with a single oprintf statement.
 */

static void step3(const ReportData& report, unsigned year, unsigned quarter)
{
    std::cout << "Yearly report for " << std::setw(4) << year << "Q" << quarter << '\n' <<
        std::setw(72) << std::setfill('-') << "" << std::setfill(' ') << '\n';

    unsigned lineNumber = 0;

    for (const auto& customer : report) {
        lineNumber++;

        oprintf("%3d %5d %-20s%10.2f     %[%6.2f%%%]\n",
        //       ^^^                                format for the line number
        //           ^^^                            format the customer number
        //               ^^^^^                      format the customer name
        //                    ^^^^^                 format the quarterly total
        //                               ^^^^^^^^^^ format the monthly data
                lineNumber,
                std::get<0>(customer), std::get<1>(customer), std::get<2>(customer),
                std::get<3>(customer));
    }

    std::cout << std::setw(72) << std::setfill('-') << "" << std::setfill(' ') << '\n';
}

/* the function step4 replaces the formating of the customer data
 *
 *      oprintf("%5d %-20s%10.2f     %[%6.2f%%%]\n", std::get<0>(customer),
 *              std::get<1>(customer), std::get<2>(customer), std::get<3>(customer));
 *
 * with a tuple format.
 *
 * A tuple format is defined as '%< f0 f1 f2... %> where each fn is the
 * format for the correspomding element in the tuple.
 */

static void step4(const ReportData& report, unsigned year, unsigned quarter)
{
    std::cout << "Yearly report for " << std::setw(4) << year << "Q" << quarter << '\n' <<
        std::setw(72) << std::setfill('-') << "" << std::setfill(' ') << '\n';

    unsigned lineNumber = 0;

    for (const auto& customer : report) {
        oprintf("%3d %<%5d %-20s%10.2f     %[%6.2f%%%]\n%>", lineNumber, customer);
        //       ^^^                                       format for the line number
        //           ^^                                    start of tuple format
        //             ^^^                                 format for element 0
        //                 ^^^^^                           format for element 1
        //                      ^^^^^                      format for element 2
        //                                 ^^^^^^^^^^^     format for element 3
        //                                              ^^ end of tuple format
    }

    std::cout << std::setw(72) << std::setfill('-') << "" << std::setfill(' ') << '\n';
}

/* the function step5 replaces the loop
 *
 *  for (const auto& customer : report) {
 *      oprintf("%<%5d %-20s%10.2f     %[%6.2f%%%]\n%>", customer);
 *  }
 *
 * with another container format
 *
 * It also uses the %N format to print the index in the container,
 * which is equal to the line number.
 */

static void step5(const ReportData& report, unsigned year, unsigned quarter)
{
    std::cout << "Yearly report for " << std::setw(4) << year << "Q" << quarter << '\n' <<
        std::setw(72) << std::setfill('-') << "" << std::setfill(' ') << '\n';

    oprintf("%[%3N %<%5d %-20s%10.2f     %[%6.2f%%%]\n%>%]", report);
    //       ^^                                     ^^   another container format
    //         ^^^                                       format for the index
    //             ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ format repeated for each element

    std::cout << std::setw(72) << std::setfill('-') << "" << std::setfill(' ') << '\n';
}

/*
 * The function step6 replaces the code to generate the header
 *
 *  std::cout << "Yearly report for " << std::setw(4) << year << "Q" << quarter << '\n' <<
 *      std::setw(72) << std::setfill('-') << "" << std::setfill(' ') << '\n';
 *
 * and the footer
 *
 *  std::cout << std::setw(72) << std::setfill('-') << "" << std::setfill(' ') << '\n';
 *
 * with oprintf statements
 *
 * In these statements, a repeat format is used.
 * A repeat format is defined as '%nn{ xxxx %}' were xxxx is the part that needs
 * repeating and nn is the repeat count.
 */

static void step6(const ReportData& report, unsigned year, unsigned quarter)
{
    oprintf("Yearly report for %dQ%d\n%72{-%}\n", year, quarter);
    //                                ^^^^^^^ generate 72 dashes.

    oprintf("%[%3N %<%5d %-20s%10.2f     %[%6.2f%%%]\n%>%]", report);

    oprintf("%72{-%}\n");
    //       ^^^^    start of repeat format; repeats 72 times
    //           ^   part to repeat
    //            ^^ end of repeat format
}

/*
 * function withPrintf combines the 3 formats
 *
 *  oprintf("Yearly report for %dQ%d\n%72{-%}\n", year, quarter);
 *  oprintf("%[%<%5d %-20s%10.2f     %[%6.2f%%%]\n%>%]", report);
 *  oprintf("%72{-%}\n");
 *
 * into one.
 */

static void withPrintf(const ReportData& report, unsigned year, unsigned quarter)
{
    oprintf("Yearly report for %dQ%d\n"
            "%72{-%}\n%[%3N %<%5d %-20s%10.2f     %[%6.2f%%%]\n%>%]%72{-%}\n",
            year, quarter, report);
}

/*
 * The function overview shows the same oprintf statement separated into
 * its different parts with a description of each part.
 *
 * for complex formats, it is helpful to write them in this way.
 */

static void overview(const ReportData& report, unsigned year, unsigned quarter)
{
    oprintf("Yearly report for %dQ%d\n" // title
            "%72{-%}"                   // 72 dashes and a new line 
            "\n"                        // new line
            "%["                        // start report vector
              "%3N "                    // container index
              "%<"                      // start customer tuple
                "%5d "                  // customer number
                "%-20s"                 // customer name
                "%10.2f     "           // quarterly total
                "%["                    // start monthly array
                  "%6.2f%%"             // monthly percentage
                "%]"                    // end monthly array"
                "\n"                    // new line
              "%>"                      // end customer tuple
            "%]"                        // end report vector
            "%72{-%}"                   // 72 dashes
            "\n",                       // new line
            year, quarter, report);     // arguments
}

/*
 * The function withFmt shows the generation of the same report using
 * std::ostream formating, but with the fmt manipulator to set the ostream flags
 *
 * The 'fmt()' at the end resets all ostream flags to their default values.
 */

static void withFmt(const ReportData& report, unsigned year, unsigned quarter)
{
    using tsio::fmt;

    std::cout << "Yearly report for " << fmt("%4d") << year << "Q" << quarter << '\n' <<
        fmt("%-\"-72s") << "" << '\n';

    unsigned lineNumber = 0;

    for (const auto& customer : report) {
        lineNumber++;

        std::cout << fmt("%3d") << lineNumber << " " <<
            fmt("%5d") << std::get<0>(customer) << " " <<
            fmt("%-20s") << std::get<1>(customer) << 
            fmt("%10.2f") << std::get<2>(customer) <<
            "     ";

        for (const auto& monthly : std::get<3>(customer)) {
            std::cout << fmt("%6.2f") << monthly << '%';
        }

        std::cout << '\n';
    }

    std::cout << fmt("%-\"-72s") << "" << '\n' << fmt();
}

int main()
{
    auto report = setUp();

    oprintf("ostreams:\n\n");
    withOstream(report, 2018, 1);

    oprintf("\n\n\nstep1:\n\n");
    step1(report, 2018, 1);

    oprintf("\n\n\nstep2:\n\n");
    step2(report, 2018, 1);

    oprintf("\n\n\nstep3:\n\n");
    step3(report, 2018, 1);

    oprintf("\n\n\nstep4:\n\n");
    step4(report, 2018, 1);

    oprintf("\n\n\nstep5:\n\n");
    step5(report, 2018, 1);

    oprintf("\n\n\nstep6:\n\n");
    step6(report, 2018, 1);

    oprintf("\n\n\noprintf:\n\n");
    withPrintf(report, 2018, 1);

    oprintf("\n\n\noverview:\n\n");
    overview(report, 2018, 1);

    oprintf("\n\n\nfmt io manipulator:\n\n");
    withFmt(report, 2018, 1);

    oprintf("\n\n");
}
