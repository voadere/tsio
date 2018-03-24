#include "tsio.h"

#include <iostream>
#include <ctime>

/*
 * This is an example that shows how to use customized formating.
 */

using namespace tsio;

class TimeFormatter
{
    public:
        TimeFormatter(const struct tm& c)
            : mTime(c)
        {
        }


        std::string format(SingleFormat fmt, int value) const
        {
            std::string result;

            fmt.setSpecifier('d');
            fmt.asprintf(result, value);
            return result;
        }

        std::string format(SingleFormat fmt, const char* value) const
        {
            std::string result;

            fmt.setSpecifier('s');
            fmt.asprintf(result, value);
            return result;
        }

        std::tuple<bool, std::string> format(SingleFormat fmt)
        {
            std::string result;

            if (fmt.getAlternative()) {
                fmt.setAlternative(false);

                switch(fmt.getSpecifier()) {
                    case 'y': result = format(fmt, mTime.tm_year % 100); break;
                    case 'm': result = format(fmt, months[mTime.tm_mon]); break;
                    case 'd': result = format(fmt, days[mTime.tm_wday]); break;
                    case 'H': result = format(fmt, mTime.tm_hour); break;

                    default: return { false, "" };
                }
            } else {
                switch(fmt.getSpecifier()) {
                    case 'y': result = format(fmt, mTime.tm_year + 1900); break;
                    case 'm': result = format(fmt, mTime.tm_mon + 1); break;
                    case 'd': result = format(fmt, mTime.tm_mday); break;
                    case 'H': result = format(fmt, mTime.tm_hour); break;
                    case 'M': result = format(fmt, mTime.tm_min); break;
                    case 'S': result = format(fmt, mTime.tm_sec); break;

                    default: return { false, "" };
                }
            }

            return { true, result };
        }

    private:
        static const char* const days[7];
        static const char* const months[12];

        const struct tm& mTime;
};

const char* const TimeFormatter::days[7] = {
    "Sunday", "Monday", "Tuesday", "Wedenesday", "Thursday", "Friday", "Saturday" };
const char* const TimeFormatter::months[12] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December" };

static TimeFormatter getFormatter(const struct tm& t)
{
    return TimeFormatter(t);
}

int main()
{
    auto theTime = std::time(nullptr);
    auto* lTime = localtime(&theTime);

    oprintf("%(%y/%m/%d %H:%M:%S%)\n", *lTime);
    oprintf("%(%02m/%02d/%#02y %02H:%02M:%02S%)\n", *lTime);
    oprintf("%(%02d/%02m/%02y %02H:%02M:%02S%)\n", *lTime);
    oprintf("\n");
    oprintf("%(%#d, %#m %d %y%)\n",*lTime);
    oprintf("%(%#3.3d, %#3.3m %d %y%)\n",*lTime);
    oprintf("\n");
}

