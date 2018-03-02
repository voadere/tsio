#include "tsio.h"

using MonthlyData = std::array<float, 3>;

// customer number, name, quarter total, monthly percentages
using CustomerData = std::tuple<unsigned, std::string, double, MonthlyData>;
using ReportData = std::vector<CustomerData>;

void withStreams(const ReportData& data)
{
}

ReportData setUp()
{
    ReportData report;
    MonthlyData monthly = { 1.2, 2.3, 3.4 };

    report.push_back(make_tuple(1, "Jetson", 123.45, monthly));

    for (auto& m : monthly) {
        m += 1.66;
    }

    report.push_back(make_tuple(3, "FlintStone", 843.99, monthly));

    for (auto& m : monthly) {
        m *= 1.23;
    }

    report.push_back(make_tuple(17, "Rubble", 1034.6,  monthly));

    return report;
}

void withOstream(const ReportData& report, unsigned year, unsigned quarter)
{
    std::cout << "Yearly report for " << std::setw(4) << year << "Q" << quarter << '\n' <<
        std::setw(72) << std::setfill('-') << "" << std::setfill(' ') << '\n';

    for (const auto& customer : report) {
        std::cout << std::setw(5) << std::get<0>(customer) << " " <<
            std::setw(20) << std::left << std::get<1>(customer) << std::right <<
            std::setw(10) << std::setprecision(2) << std::fixed << std::get<2>(customer) <<
            "     ";

        for (const auto& monthtly : std::get<3>(customer)) {
            std::cout << std::setw(6) << std::setprecision(2) << monthtly << '%';
        }

        std::cout << '\n';
    }

    std::cout << std::setw(72) << std::setfill('-') << "" << std::setfill(' ') << '\n';
}

void withFmt(const ReportData& report, unsigned year, unsigned quarter)
{
    using tsio::fmt;

    std::cout << "Yearly report for " << fmt("4d") << year << "Q" << quarter << '\n' <<
        fmt("-\"-72s") << "" << '\n';

    for (const auto& customer : report) {
        std::cout << fmt("5d") << std::get<0>(customer) << " " <<
            fmt("-20s") << std::get<1>(customer) << 
            fmt("10.2f") << std::get<2>(customer) <<
            "     ";

        for (const auto& monthtly : std::get<3>(customer)) {
            std::cout << fmt("6.2f") << monthtly << '%';
        }

        std::cout << '\n';
    }

    std::cout << fmt("-\"-72s") << "" << '\n' << fmt();
}

void withPrintf(const ReportData& report, unsigned year, unsigned quarter)
{
    using tsio::oprintf;

    oprintf("Yearly report for %dQ%d\n"
            "%72{-%}\n%[%<%5d %-20s%10.2f     %[%6.2f%%%]\n%>%]%72{-%}\n",
            year, quarter, report);
}

void breakdown(const ReportData& report, unsigned year, unsigned quarter)
{
    using tsio::oprintf;

    oprintf("Yearly report for %dQ%d\n" // title
            "%72{-%}\n"                 // 72 dashes and a new line
            "%["                        // start report vector
              "%<"                      // start customer tuple
                "%5d "                  // customer number
                "%-20s"                 // customer name
                "%10.2f     "           // quarter total
                "%["                    // start monthly array
                  "%6.2f%%"             // monthly percentage
                "%]\n"                  // end monthly array
              "%>"                      // end customer tuple
            "%]"                        // end report vector
            "%72{-%}\n",                // 72 dashes and a new line
            year, quarter, report);     // arguments
}

int main()
{
    auto report = setUp();

    std::cout << "With ostreams:\n\n";
    withOstream(report, 2018, 1);
    std::cout << "\n\n\n";

    std::cout << "With fmt io manipulator:\n\n";
    withFmt(report, 2018, 1);
    std::cout << "\n\n\n";

    std::cout << "With oprintf:\n\n";
    withPrintf(report, 2018, 1);
    std::cout << "\n\n\n";

    std::cout << "With breakdown:\n\n";
    breakdown(report, 2018, 1);
}
