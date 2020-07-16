# MailMigrator
Mail migrator was created as part of a job interview. 

The tasks were to
1. Define database structure for users of groupware server according to the specified structure
2. Create an email migration tool that migrated .imap files from specified folder hierarchy into the created database

This solution is written in C++ with utilization
*	C++11
*	SQLite for the database (for portability of solution)
*	std::filesystem (in the time of creation experimental part of std, which could have been replaced by boost::filesystem in real use scenario)
*	Multithreading – With the utilization of thread pool
