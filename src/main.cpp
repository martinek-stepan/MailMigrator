#include <iostream>

#include "Migrator.h"


/*
#pragma warning( push )
#pragma warning(disable: 4251)
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#pragma warning( pop )
*/


int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: mailmigrator <fs root path> <db file path>" << std::endl;
        return 1;
    }

    Migrator m(argv[1], argv[2]);

    m.loadUsersFromFS();
    m.saveUsersToDB();
    m.migrateMails();

    return 0;
}