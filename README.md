📂 Database System Using C and Shell Script
🔍 Overview
This project implements a SQL-based command-line database system that integrates:

C programming for core logic and command parsing.

Shell scripting for user authentication, logging, and file management.

Key features include:

User authentication and logging via a shell script.

A database file storing the database contents.

Role-based access control:

Administrators can create the database file and users.

Normal users cannot create/delete the database file but can create SQL script files and run queries to modify data.

A custom database shell for interacting with the system.

🧩 Features
🔐 Authentication & Users
Upon system startup, an authentication function runs to manage users and record activity in a log file.

Default administrator user:

Username: SYS

Password: Set by the first user during setup.

Only administrators can create new users with the CREATE USER command.

💻 Database Shell Commands
CREATE USER: (Admin only) Create new database users.

PERFORM filename: Opens a new window to write SQL queries into a file.

COMPILE filename: Executes SQL queries written in the file.

LOGOUT: Logs out the current user.

EXIT: Exits the database shell.

Invalid commands generate alert entries in the log file.

🗃️ SQL Query Support in Files
INSERT INTO table_name VALUES(...);
Adds new records to the specified table.

UPDATE INTO table_name SET ... WHERE ...;
Updates specified records, with optional WHERE clause to filter rows.

SELECT ... FROM table_name WHERE ... ORDER BY ... GROUP BY ... HAVING ...;
Supports selecting attributes, filtering with WHERE, sorting with ORDER BY, grouping with GROUP BY, and filtering groups with HAVING.

DELETE FROM table_name WHERE ...;
Deletes records; without a WHERE clause deletes all records.

🔒 Concurrency Control
The system uses file locking mechanisms to support concurrency control and avoid data corruption during simultaneous operations.

⚙️ How to Run

Compile the C program:
gcc user.c -o user

Run the shell script to start the database shell:
bash ./sample.sh
