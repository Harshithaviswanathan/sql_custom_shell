# 📂 Database System Using C and Shell Script

## 🔍 Overview

This project implements a **simple SQL-based command-line database system** that integrates:

- ✅ **C programming** – for core logic, SQL parsing, and execution.
- ✅ **Shell scripting** – for user authentication, logging, and session control.

This system supports **basic SQL operations**, role-based user access, and file-level concurrency control using locks.

---

## 🧩 Features

### 🔐 Authentication & User Roles

- Runs an authentication function on startup to manage users and track activity in a `log.txt` file.
- Supports **two types of users**:
  - **Administrator** (`SYS`) — has full privileges including user creation and database file management.
  - **Normal User** — can only run SQL queries via script files.
- **Default Admin**:
  - **Username**: `SYS`
  - **Password**: Defined by the first user at setup.
- Admins can create users using the `CREATE USER` command.

---

### 💻 Database Shell Commands

| Command             | Description                                              | Access         |
|---------------------|----------------------------------------------------------|----------------|
| `CREATE USER`       | Create new users                                         | Admin only     |
| `PERFORM filename`  | Opens a new file to write SQL queries                    | All users      |
| `COMPILE filename`  | Executes the SQL queries written in a given file         | All users      |
| `LOGOUT`            | Logs out the current user                                | All users      |
| `EXIT`              | Exits the shell                                          | All users      |

> ⚠️ Any invalid operation generates a log entry with an alert message.

---

### 🗃️ SQL Query Support in Files

Supports the following SQL commands inside `.sql` files:
📥 INSERT
INSERT INTO table_name VALUES(value1, value2, ...);
✏️ UPDATE
UPDATE INTO table_name SET column=value WHERE condition;
🔎 SELECT
SELECT column1, column2 FROM table_name 
WHERE condition 
ORDER BY column 
GROUP BY column 
HAVING condition;
🗑️ DELETE
DELETE FROM table_name WHERE condition;
🔒 Concurrency Control
Implements file-level locking mechanisms using C to prevent race conditions and data corruption when multiple users access or modify the database simultaneously.

⚙️ How to Run
Create userlog.txt file and insert the username:password for admin
echo "admin:adminpassword" > userlog.txt
1️⃣ Compile the C Program

gcc final_new.c -o final_new
2️⃣ Run the Shell Script

bash ./database.sh
