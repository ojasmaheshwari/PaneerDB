#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <memory>

class Statement;
class DiskManager;
class BufferPoolManager;
class Catalog;

class Engine {
public:
    Engine();
    ~Engine();

    void useDatabase(const std::string& name);
    void createDatabase(const std::string& name);
    void execute(Statement* statement);
    bool isDbActive() const;
    std::string getActiveDatabase() const;

private:
    bool active;
    std::string activeDbName;
    
    std::unique_ptr<DiskManager> diskManager;
    std::unique_ptr<BufferPoolManager> bpm;
    std::unique_ptr<Catalog> catalog;
};

#endif // ENGINE_H
