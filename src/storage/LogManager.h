#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include "LogRecord.h"
#include <string>
#include <fstream>
#include <mutex>

class LogManager {
public:
    explicit LogManager(const std::string& log_file_name);
    ~LogManager();

    // Appends a log record to the log file
    void AppendLogRecord(const LogRecord& record);

    // Flushes the log file to disk
    void Flush();

private:
    std::string log_file_name_;
    std::ofstream log_file_;
    std::mutex latch_;
};

#endif // LOG_MANAGER_H
