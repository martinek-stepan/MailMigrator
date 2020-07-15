#pragma once

#include <atomic>

#include "utils.h"
#include "SQLiteCpp/SQLiteCpp.h"

enum
{
    TYPE_EMAIL = 1,
    TYPE_CONTACT,
    TYPE_CALENDAR
};

class Migrator : private Semaphore
{
public:
    Migrator(fs::path fsRootPath, std::string dbPath);
    ~Migrator();

    void loadUsersFromFS();

    void userLoader(std::atomic<bool> const* run);
    void mailTransferer(std::atomic<bool> const* run);
    
    void saveUsersToDB();
    void migrateMails();
private:
    std::vector<class User*> _users;
    std::vector<fs::path> _domains;
    std::mutex _critic_lock;
    std::mutex _db_lock;
    fs::path _fsRootPath;
    std::string _dbPath;
    SQLite::Database _db;
    uint8 _nrThreads;

};
