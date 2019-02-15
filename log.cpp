#include "log.h"

#include <stdio.h>
#include <sys/time.h>

inline std::string NowTime();

Log::Log()
{
}

std::ostringstream& Log::Get(TLogLevel level)
{
    os << "- " << NowTime();
    os << " " << ToString(level) << ": ";
    os << '\t';
    return os;
}

Log::~Log()
{
    os << std::endl;
   fprintf(stderr, "%s", os.str().c_str());
   fflush(stderr);
}

TLogLevel& Log::ReportingLevel()
{
    static TLogLevel reportingLevel = DEBUG4;
    return reportingLevel;
}

std::string Log::ToString(TLogLevel level)
{
    static const char* const buffer[] = {"ERROR", "WARNING", "INFO", "DEBUG", "DEBUG1", "DEBUG2", "DEBUG3", "DEBUG4"};
    return buffer[level];
}

inline std::string NowTime()
{
    char buffer[11];
    time_t t;
    time(&t);
    tm r = {};
    strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
    struct timeval tv;
    gettimeofday(&tv, 0);
    char result[100] = {0};
    std::sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000); 
    return result;
}
