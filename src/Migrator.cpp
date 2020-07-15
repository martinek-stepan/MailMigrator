#include "Migrator.h"
#include "ThreadPool.h"
#include "User.h"
#include "sqlite3.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

Migrator::Migrator(fs::path fsRootPath, std::string dbPath)
    : _fsRootPath(fsRootPath)
    , _dbPath(dbPath)
    , _db(_dbPath, SQLITE_OPEN_READWRITE)
{
}

Migrator::~Migrator()
{
    for (auto* user : _users)
        delete user;

    _users.clear();
}

void Migrator::loadUsersFromFS()
{
    std::cout << "Loading domains and users..." << std::endl;
    // start thread pool where every thread create users for domains retrieved from master
    ThreadPool pool(_nrThreads, &Migrator::userLoader, this);

    // Loot through domain directories in root
    for (auto& p : fs::directory_iterator(_fsRootPath))
    {
        if (!fs::is_directory(p))
            continue;

        Guard guad(_critic_lock);
        _domains.push_back(p);
        _sem_value++;
        _sem_condition.notify_one();
    }

}

void Migrator::userLoader(std::atomic<bool> const* run)
{
    std::atomic<bool> const& _run = *run;
    fs::path domain;
    std::vector<User*> users;

    // run until we have are asked to stop
    do
    {
        // Scope for critic section so guard holds lock only necessary time
        {
            // Prevent more than one thread in this section
            Guard guard(_sem_lock);

            // Wait until we have something to do or job is done
            while (_sem_value == 0 && _run)
                _sem_condition.wait(guard);

            if (!_run && _sem_value == 0)
                break;

            {
                Guard g(_critic_lock);
                domain = _domains.back();
                _domains.pop_back();
                _sem_value--;
                //std::cout << std::this_thread::get_id() << " " << domain.string() << std::endl;
            }
        }

        // Run through user directories in current domain
        for (auto& p : fs::directory_iterator(domain))
            if (fs::is_directory(p))
                users.push_back(new User(p.path(), domain.filename().string()));

    } while (_run || _sem_value > 0);

    // Insert to vector containing all users
    {
        Guard guard(_critic_lock);
        _users.insert(_users.end(), users.begin(), users.end());
    }

    // Notify all threads that still waiting to end their work
    _sem_condition.notify_all();
}

void Migrator::saveUsersToDB()
{
    std::cout << "Saving users into DB and retrieving their IDs..." << std::endl;
    try
    {
        // Begin transaction
        SQLite::Transaction transaction(_db);
        for (auto const* user : _users)
        {
            SQLite::Statement  q1(_db, "INSERT INTO users (username, domain) VALUES (?,?)");
            q1.bind(1, user->usernameStr);
            q1.bind(2, user->domain);
            q1.exec();
        }
        // Commit transaction
        transaction.commit();

        // Compile a SQL query
        SQLite::Statement  q2(_db, " SELECT u.id,u.username,u.domain, f.id FROM users u JOIN user_folders f ON  u.id = f.fkey_user WHERE f.fkey_parent_folder IS NULL AND f.fkey_type = ?");
        q2.bind(1, TYPE_EMAIL);

        // Loop to execute the query step by step, to get rows of result
        while (q2.executeStep())
        {
            // Demonstrate how to get some typed column value
            uint32 id = q2.getColumn(0);
            std::string username = q2.getColumn(1);
            std::string domain = q2.getColumn(2);
            uint32 root_folder = q2.getColumn(3);

            // Find correct user
            auto itr = std::find_if(_users.begin(), _users.end(), [&username, &domain](User const* user)
            {
                return username == user->usernameStr && domain == user->domain;
            });

            // If found assign id
            if (itr != _users.end())
            {
                (*itr)->id = id;
                (*itr)->root_folder = root_folder;
            }
        }
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }

}

void Migrator::migrateMails()
{
    std::cout << "Starting mail migration, this may take a while..." << std::endl;

    // Initialize semaphore value
    _sem_value = (uint32)_users.size();

    ThreadPool pool(_nrThreads, &Migrator::mailTransferer, this);
}


void Migrator::mailTransferer(std::atomic<bool> const* run)
{
    std::atomic<bool> const& _run = *run;
    do 
    {
        User* user;
        {
            // Prevent more than one thread in this section
            Guard guard(_sem_lock);

            // Wait until we have something to do or job is done
            while (_sem_value == 0 && _run)
                _sem_condition.wait(guard);

            if (!_run && _sem_value == 0)
                break;

            {
                Guard g(_critic_lock);
                user = _users.back();
                _users.pop_back();
                _sem_value--;
                std::cout << "Remaining users: " << (_sem_value +1)<< "\r";
            }
        }
        if (user->id == 0)
        {
            delete user;
            continue;
        }

        std::unordered_map<std::string, uint32> folders;
        folders[""] = user->root_folder;

        for (auto& p : fs::recursive_directory_iterator(user->username))
        {
            if (fs::is_directory(p))
            {
                std::string const& parent_path = p.path().parent_path().string();
                size_t found = parent_path.find(user->username.string());
                
                if (found == 0)
                {
                    std::string short_path = parent_path.substr(user->username.string().size());

                    auto itr = folders.find(short_path);
                    if (itr == folders.end())
                        continue;

                    uint32 parentId = itr->second;
                    std::string filename = p.path().filename().string();
                    try
                    {
                        Guard guard(_db_lock);
                        SQLite::Statement   q1(_db, "INSERT INTO user_folders (name,fkey_parent_folder, fkey_user, fkey_type) VALUES(?,?,?,?)");
                        q1.bind(1, filename);
                        q1.bind(2, parentId);
                        q1.bind(3, user->id);
                        q1.bind(4, TYPE_EMAIL);
                        q1.exec();

                        SQLite::Statement   q2(_db, "SELECT id FROM user_folders WHERE fkey_parent_folder = ? AND fkey_user = ? AND name = ?");
                        q2.bind(1, parentId);
                        q2.bind(2, user->id);
                        q2.bind(3, filename);

                        // Loop to execute the query step by step, to get rows of result
                        while (q2.executeStep())
                        {
                            folders[p.path().string().substr(user->username.string().size())] = q2.getColumn(0);
                            break;
                        }
                    }
                    catch (std::exception& e)
                    {
                        std::cout << e.what() << std::endl;
                    }

                }

                continue;
            }
            
            std::ifstream file(p.path());
            std::stringstream ss; 
            std::string header;
            std::string line;
            bool completeHeader = false;
            while (getline(file, line))
            {
                if (!completeHeader && line.empty())
                {
                    completeHeader = true;
                    header = ss.str();
                    ss.str("");
                    continue;
                }
                ss << line;
            }
            auto itr = folders.find(p.path().parent_path().string().substr(user->username.string().size()));
            if (itr != folders.end())
            {
                Guard guard(_db_lock);
                SQLite::Statement query(_db, "INSERT INTO user_emails (header, body, fkey_user, fkey_folder) VALUES(?,?,?,?)");
                query.bind(1, header);
                query.bind(2, ss.str());
                query.bind(3, user->id);
                query.bind(4, itr->second);
                query.exec();
            }
        }

        delete user;
    } while (_run || _sem_value > 0);

    _sem_condition.notify_all();
}
