#include "LogManager.h"
#include <iostream>

LogManager::LogManager(const std::string& log_file_name) 
    : log_file_name_(log_file_name) {
    log_file_.open(log_file_name_, std::ios::app | std::ios::binary);
    if (!log_file_.is_open()) {
        std::cerr << "Error: Could not open log file " << log_file_name_ << "\n";
    }
}

LogManager::~LogManager() {
    if (log_file_.is_open()) {
        Flush();
        log_file_.close();
    }
}

void LogManager::AppendLogRecord(const LogRecord& record) {
    std::lock_guard<std::mutex> lock(latch_);
    if (!log_file_.is_open()) return;

    // Serialize type and txn_id
    int32_t type = static_cast<int32_t>(record.type);
    log_file_.write(reinterpret_cast<const char*>(&type), sizeof(type));
    log_file_.write(reinterpret_cast<const char*>(&record.txn_id), sizeof(record.txn_id));

    if (record.type == LogRecordType::INSERT || record.type == LogRecordType::DELETE) {
        // Serialize table_name
        uint32_t table_name_len = record.table_name.length();
        log_file_.write(reinterpret_cast<const char*>(&table_name_len), sizeof(table_name_len));
        log_file_.write(record.table_name.c_str(), table_name_len);

        // Serialize page_id and slot_id
        log_file_.write(reinterpret_cast<const char*>(&record.page_id), sizeof(record.page_id));
        log_file_.write(reinterpret_cast<const char*>(&record.slot_id), sizeof(record.slot_id));

        // Serialize tuple_data
        uint32_t tuple_len = record.tuple_data.length();
        log_file_.write(reinterpret_cast<const char*>(&tuple_len), sizeof(tuple_len));
        log_file_.write(record.tuple_data.c_str(), tuple_len);
    }
}

void LogManager::Flush() {
    std::lock_guard<std::mutex> lock(latch_);
    if (log_file_.is_open()) {
        log_file_.flush();
    }
}
