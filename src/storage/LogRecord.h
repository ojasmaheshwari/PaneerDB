#ifndef LOG_RECORD_H
#define LOG_RECORD_H

#include <string>
#include <cstdint>
#include <vector>

enum class LogRecordType {
    BEGIN = 0,
    COMMIT,
    ABORT,
    INSERT,
    DELETE
};

struct LogRecord {
    int txn_id;
    LogRecordType type;
    std::string table_name;
    int32_t page_id;
    int32_t slot_id;
    std::string tuple_data;

    // Constructors for different record types
    LogRecord(int txn_id, LogRecordType type) 
        : txn_id(txn_id), type(type), page_id(-1), slot_id(-1) {}
        
    LogRecord(int txn_id, LogRecordType type, const std::string& table_name, int32_t page_id, int32_t slot_id, const std::string& tuple_data)
        : txn_id(txn_id), type(type), table_name(table_name), page_id(page_id), slot_id(slot_id), tuple_data(tuple_data) {}
};

#endif // LOG_RECORD_H
