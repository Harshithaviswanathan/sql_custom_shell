
user_exists() {
    grep -q "^$1:$2$" userlog.txt
}


add_user() {
    echo "Enter new username:"
    read new_username
    echo "Enter password for $new_username:"
    read -s new_password
    echo "$new_username:$new_password" >> userlog.txt
    echo "User $new_username added successfully"
}


create_database() {
    echo "Enter the name for the new database directory:"
    read db_name
    mkdir "$db_name"
    echo "$db_name" >> admin_databases.txt
    echo "Database directory $db_name created successfully"
}

display_database_info() {
    echo "Databases created by the admin:"
    cat admin_databases.txt
    echo "-------------------------"

    while IFS= read -r database; do
        echo "Database: $database"
        cd "$database" || continue

        # List users
        echo "Users:"
        ls -d */ | sed 's#/##'

        # List tables for each user
        for user_dir in */; do
            echo "Tables for ${user_dir%/}:"
            cd "$user_dir" || continue
            ls
            cd ..
        done

        cd ..
        echo "-------------------------"
    done < admin_databases.txt
}


# Function to run the SQL shell program
run_sql_shell() {
    if [ -f "final_new" ]; then
        ./final_new "$1" "$2"
    else
        echo "SQL shell program 'final' not found"
    fi
}

# Function to display menu for admin
admin_menu() {
    echo "Admin Menu:"
    echo "1) Add a new user"
    echo "2) Create a new database"
    echo "3) Display users and tables in each database"
    echo "4) Exit"
}

# Function to display menu for user
user_menu() {
    echo "User Menu:"
    echo "1) Select database"
    echo "2) Exit"
}


display_databases() {
    echo "Available databases:"
    cat admin_databases.txt
}

# Function to handle database selection by the user
select_database() {
    display_databases
    echo "Enter the name of the database:"
    read selected_database
    if grep -q "^$selected_database$" admin_databases.txt; then
        run_sql_shell "$selected_database" "$username"
    else
        echo "Database '$selected_database' does not exist or you don't have access to it"
    fi
}

# Main function
main() {
    echo "Are you an admin or a user? (admin/user)"
    read user_type

    if [ "$user_type" = "admin" ]; then
        echo "Enter admin username:"
        read admin_username
        echo "Enter password for $admin_username:"
        read -s admin_password

        # Check if admin credentials are correct
        if user_exists "$admin_username" "$admin_password"; then
            echo "Welcome, $admin_username!"
            while true; do
                admin_menu
                read choice
                case $choice in
                    1) add_user ;;
                    2) create_database ;;
                    3) display_database_info ;;
                    4) break ;;
                    *) echo "Invalid choice";;
                esac
            done
        else
            echo "Incorrect admin username or password"
        fi
    elif [ "$user_type" = "user" ]; then
        echo "Enter username:"
        read username
        echo "Enter password for $username:"
        read -s password

        # Check if user credentials are correct
        if user_exists "$username" "$password"; then
            echo "Welcome, $username!"
            while true; do
                user_menu
                read choice
                case $choice in
                    1) select_database ;;
                    2) exit ;;
                    *) echo "Invalid choice";;
                esac
            done
        else
            echo "Incorrect username or password"
        fi
    else
        echo "Invalid user type"
    fi
}

# Check if admin_databases.txt exists, if not, create it
if [ ! -f "admin_databases.txt" ]; then
    touch admin_databases.txt
fi

main
