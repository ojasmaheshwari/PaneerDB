#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <memory>

class Statement;
class DeleteStatement;
class DiskManager;
class BufferPoolManager;
class Catalog;

class Engine {
public:
    Engine();
    ~Engine();

    void execute(Statement* statement);
    bool isDbActive() const;
    std::string getActiveDatabase() const;

private:
    void executeInsert(class InsertStatement* stmt);
    void executeSelect(class SelectStatement* stmt);
    void executeDelete(class DeleteStatement* stmt);
    void useDatabase(const std::string& name);
    void createDatabase(const std::string& name);

    bool active;
    std::string activeDbName;
    
    std::unique_ptr<DiskManager> diskManager;
    std::unique_ptr<BufferPoolManager> bpm;
    std::unique_ptr<Catalog> catalog;
    
    std::unique_ptr<class LockManager> lockManager;
    std::unique_ptr<class TransactionManager> txnManager;
    std::unique_ptr<class LogManager> logManager;
    class Transaction* currentTxn = nullptr;
};

#endif // ENGINE_H

