#ifndef __LOG_H__
#define __LOG_H__

#include <sstream>
#include <string>

#define LOG(level) \
  if (level > Log::ReportingLevel()) ; \
  else Log().Get(level)

enum TLogLevel {ERROR, WARNING, INFO, DEBUG, DEBUG1, DEBUG2, DEBUG3, DEBUG4};
TLogLevel& operator++(TLogLevel& level);

class Log
{
public:
    Log();
    virtual ~Log();
    std::ostringstream& Get(TLogLevel level = INFO);
public:
    static TLogLevel IncrementReportingLevel();
    static TLogLevel ReportingLevel();
    static std::string ToString(TLogLevel level);
protected:
    std::ostringstream os;
private:
    static TLogLevel logLevel;
    Log(const Log&);
    Log& operator =(const Log&);
};

#endif //__LOG_H__
