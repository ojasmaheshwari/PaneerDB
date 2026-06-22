#ifndef ENGINE_H
#define ENGINE_H

#include <string>

class Statement;

class Engine {
public:
    Engine();

    void useDatabase(const std::string& name);
    void createDatabase(const std::string& name);
    void execute(Statement* statement);
    bool isDbActive() const;
    std::string getActiveDatabase() const;

private:
    bool active;
    std::string activeDbName;
};

#endif // ENGINE_H
