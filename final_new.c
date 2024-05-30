#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/file.h>

#define MAXCOM 1000
#define MAXLIST 100
#define MAXTABLES 10

char* databaseDir;
char* fileName;
int DoesTableLoaded = 0;
char* currentUsername;

typedef struct {
    char columns[MAXLIST][100];
} Row;

typedef struct {
    char name[100];
    char columns[MAXLIST][100];
    int num_columns;
    Row rows[MAXLIST];
    int num_rows;
} Table;

typedef struct {
    Table tables[MAXTABLES];
    int num_tables;
} Database;

Database current_database;

void clear() {
    printf("\033[H\033[J");
}

int takeInput(char* str) {
    char* buf;

    buf = readline("\nSQL> ");
    if (strlen(buf) != 0) {
        add_history(buf);
        strcpy(str, buf);
        return 0;
    } else
        return 1;
}

void create_fileName(char* parsed) {
    fileName = (char*)malloc(sizeof(char) * 100);
    sprintf(fileName, "%s/%s/%s.dat", databaseDir, currentUsername, parsed);
}

int LoadTable(char* tableName) {

        create_fileName(tableName);

    FILE* f;
    f = fopen(fileName, "rb");
    if (!f) {
        perror("Table Not found!");
        return -1;
    }
    if(flock(fileno(f),LOCK_SH) < 0){
    perror("FILE LOCK [CREATE TABLE]");
    exit(-1);
    }

    int bytes, len = 2, j = 0, i = 0;
    char* c = (char*)malloc(1);
    char* result = (char*)malloc(1);
    result[0] = '\0';

    Table new_table;

    char* name = (char*)malloc(sizeof(char) * 100);
    sprintf(name, "%s.dat", tableName);

    strcpy(new_table.name, name);
    new_table.num_columns = 0;
    new_table.num_rows = 0;
    free(name);

    while (!feof(f)) {
        bytes = fread(c, sizeof(char), 1, f);
        c[bytes] = '\0';
        if (!strcmp(c, ",")) {
            if (!i)
                strcpy(new_table.columns[j], result);
            else {
                strcpy(new_table.rows[i].columns[j], result);
            }
            result[0] = '\0';
            len = 1;
            j++;
        } else if (!strcmp(c, "\n")) {
            if (!i) {
                strcpy(new_table.columns[j], result);
                new_table.num_columns = j + 1;
            } else
                strcpy(new_table.rows[i].columns[j], result);
            result[0] = '\0';
            len = 1;
            j = 0;
            i++;
            new_table.num_rows = i;
        } else {
            result = (char*)realloc(result, (++len));
            sprintf(result, "%s%s", result, c);
        }
    }

    current_database.tables[current_database.num_tables++] = new_table;

    flock(fileno(f),LOCK_UN);
    fclose(f);
    free(result);
    free(c);

    return 0;
}



void createTable(char** parsed) {
    printf("Creating table %s...\n", parsed[2]);

    if (current_database.num_tables >= MAXTABLES) {
        printf("Error: Maximum number of tables reached\n");
        return;
    }

    Table new_table;
    sprintf(new_table.name, "%s.dat", parsed[2]);
    new_table.num_columns = 0;
    new_table.num_rows = 0;

    int i = 3;
    char* result = (char*)malloc(1);
    result[0] = '\0';
    while (parsed[i] != NULL) {
        result = (char*)realloc(result, strlen(result) + strlen(parsed[i]) + 2);
        strcat(result, parsed[i]);
        if (parsed[i + 1] != NULL)
            strcat(result, ",");
        strcpy(new_table.columns[new_table.num_columns], parsed[i]);
        new_table.num_columns++;
        i++;
    }
    sprintf(result, "%s\n", result);

    create_fileName(parsed[2]);

    printf("fileName : %s\n",fileName);

    FILE* f;
    f = fopen(fileName, "wb");

    if (!f) {
        perror("Table Not Found [ CREATE TABLE ]");
        exit(-1);
    }

    if(flock(fileno(f),LOCK_EX) < 0){
    perror("FILE LOCK [CREATE TABLE]");
    exit(-1);
    }
    // Write column names to the file
    fwrite(result, sizeof(char) * (strlen(result)), 1, f);

    flock(fileno(f),LOCK_UN);
    fclose(f);

    current_database.tables[current_database.num_tables++] = new_table;

    printf("Columns: ");
    for (int j = 0; j < new_table.num_columns; j++) {
        printf("%s ", new_table.columns[j]);
    }
    printf("\nTable created successfully!\n");
}


void insertInto(char** parsed) {
    printf("Inserting into table %s...\n", parsed[2]);

    char tab[20];
    sprintf(tab,"%s.dat",parsed[2]);
    DoesTableLoaded = 0;

    for(int i=0;i<current_database.num_tables;i++){
        if(!strcmp(tab,current_database.tables[i].name)){
            DoesTableLoaded = 1;
            break;
        }
    }

    if (!DoesTableLoaded) {
        LoadTable(parsed[2]);
        DoesTableLoaded = 1;
    }

    create_fileName(parsed[2]);

    if (current_database.num_tables == 0) {
        printf("Error: No tables created yet\n");
        return;
    }

    Table* target_table = NULL;
    char* table_name = (char*)malloc(sizeof(char) * 100);
    for (int i = 0; i < current_database.num_tables; i++) {
        sprintf(table_name, "%s.dat", parsed[2]);
        if (strcmp(table_name, current_database.tables[i].name) == 0) {
            target_table = &current_database.tables[i];
            break;
        }
    }

    free(table_name);

    if (target_table == NULL) {
        printf("Error: Table %s not found\n", parsed[2]);
        return;
    }

    if (target_table->num_columns == 0) {
        printf("Error: Table %s has no columns\n", parsed[2]);
        return;
    }

    if (target_table->num_rows >= MAXLIST) {
        printf("Error: Maximum number of rows reached for table %s\n", parsed[2]);
        return;
    }

    char* result = (char*)malloc(1);

    for (int i = 0; i < target_table->num_columns; i++) {
        result = (char*)realloc(result, sizeof(char) * (strlen(parsed[i + 3]) + 1));
        strcpy(target_table->rows[target_table->num_rows].columns[i], parsed[i + 3]);

        if (!i)
            strcpy(result, parsed[i + 3]);
        else
            strcat(result, parsed[i + 3]);

        if (i != target_table->num_columns - 1)
            strcat(result, ",");

        printf("Inserted : %s\n", parsed[i + 3]);
    }

    result[strlen(result)] = '\n';
    printf("Result : %s\n", result);
    FILE* f;
    f = fopen(fileName, "ab");
    if (!f) {
        perror("Table Not Found!");
        exit(-1);
    }
    if(flock(fileno(f),LOCK_EX) < 0){
    perror("FILE LOCK [INSERT TABLE]");
    exit(-1);
    }
    fwrite(result, sizeof(char) * (strlen(result)), 1, f);

    flock(fileno(f),LOCK_UN);
    fclose(f);

    target_table->num_rows++;

    printf("Inserted into table %s successfully\n", parsed[2]);
    free(result);
}




void modifyRecord(char** parsed) {
    printf("Modifying record in table %s...\n", parsed[1]);

    char result[20];
    sprintf(result,"%s.dat",parsed[2]);
    DoesTableLoaded = 0;

    for(int i=0;i<current_database.num_tables;i++){
        if(!strcmp(result,current_database.tables[i].name)){
            DoesTableLoaded = 1;
            break;
        }
    }

    if (!DoesTableLoaded) {
        LoadTable(parsed[1]);
        DoesTableLoaded = 1;
    }

    create_fileName(parsed[1]);

    if (current_database.num_tables == 0) {
        printf("Error: No tables created yet\n");
        return;
    }

    Table* target_table = NULL;
    char* table_name = (char*)malloc(sizeof(char) * 100);
    for (int i = 0; i < current_database.num_tables; i++) {
        sprintf(table_name, "%s.dat", parsed[1]);
        if (strcmp(table_name, current_database.tables[i].name) == 0) {
            target_table = &current_database.tables[i];
            break;
        }
    }
    free(table_name);

    if (target_table == NULL) {
        printf("Error: Table %s not found\n", parsed[1]);
        return;
    }

    // Ensure that the parsed array contains at least 9 components
    if (!parsed[2] || !parsed[3] || !parsed[4] || !parsed[5] || !parsed[6] || !parsed[7] || !parsed[8]) {
        printf("Error: Incomplete MODIFY command\n");
        return;
    }

    char* column_name = parsed[2];
    char* new_value = parsed[4];
    char* condition_column_name = parsed[6];
    char* condition_value = parsed[8];
    //printf("Error:%s",condition_value);


    int column_index = -1;
    for (int i = 0; i < target_table->num_columns; i++) {
        if (strcmp(target_table->columns[i], column_name) == 0) {
            column_index = i;
            break;
        }
    }

    if (column_index == -1) {
        printf("Error: Column %s not found in table %s\n", column_name, parsed[1]);
        return;
    }


    int record_updated = 0;
    for (int i = 0; i < target_table->num_rows; i++) {
      for(int j=0;j < target_table->num_columns; j++) {
        if (strcmp(target_table->rows[i].columns[j], condition_value) == 0) {
            strcpy(target_table->rows[i].columns[column_index], new_value);
            record_updated = 1;
            break;
          }
        }
    }

    if (!record_updated) {
        printf("Error: Record not found with condition %s = %s\n", condition_column_name, condition_value);
        return;
    }

    FILE *f = fopen(fileName, "wb");
    if (!f) {
        perror("Error opening file for writing");
        return;
    }
    if(flock(fileno(f),LOCK_EX) < 0){
    perror("FILE LOCK [CREATE TABLE]");
    exit(-1);
    }

    for (int i = 0; i < target_table->num_rows; i++) {
        for (int j = 0; j < target_table->num_columns; j++) {
            fprintf(f, "%s", target_table->rows[i].columns[j]);
            if (j < target_table->num_columns - 1) {
                fprintf(f, ",");
            }
        }
        fprintf(f, "\n");
    }

    flock(fileno(f),LOCK_UN);
    fclose(f);
    printf("Record modified successfully\n");
}


void deleteRecord(char** parsed) {
    printf("Deleting record from table %s...\n", parsed[2]);

    char result[20];
    sprintf(result,"%s.dat",parsed[2]);
    DoesTableLoaded = 0;

    for(int i=0;i<current_database.num_tables;i++){
        if(!strcmp(result,current_database.tables[i].name)){
            DoesTableLoaded = 1;
            break;
        }
    }

    if (!DoesTableLoaded) {
        LoadTable(parsed[2]);
        DoesTableLoaded = 1;
    }

    create_fileName(parsed[2]);

    if (current_database.num_tables == 0) {
        printf("Error: No tables created yet\n");
        return;
    }

    Table* target_table = NULL;
    char* table_name = (char*)malloc(sizeof(char) * 100);
    for (int i = 0; i < current_database.num_tables; i++) {
        sprintf(table_name, "%s.dat", parsed[2]);
        if (strcmp(table_name, current_database.tables[i].name) == 0) {
            target_table = &current_database.tables[i];
            break;
        }
    }
    free(table_name);

    if (target_table == NULL) {
        printf("Error: Table %s not found\n", parsed[2]);
        return;
    }


    if (!parsed[3] || !parsed[4] || !parsed[5] || !parsed[6]) {
        printf("Error: Incomplete DELETE command\n");
        return;
    }

    char* condition_column_name = parsed[4];
    char* condition_value = parsed[6];


    int condition_column_index = -1;
    for (int i = 0; i < target_table->num_columns; i++) {
        if (strcmp(target_table->columns[i], condition_column_name) == 0) {
            condition_column_index = i;
            break;
        }
    }

    if (condition_column_index == -1) {
        printf("Error: Condition column %s not found in table %s\n", condition_column_name, parsed[2]);
        return;
    }


    int records_deleted = 0;
    for (int i = 0; i < target_table->num_rows; i++) {
        if (strcmp(target_table->rows[i].columns[condition_column_index], condition_value) == 0) {
            for (int j = i; j < target_table->num_rows - 1; j++) {
                memcpy(&target_table->rows[j], &target_table->rows[j + 1], sizeof(Row));
            }
            target_table->num_rows--;
            i--;
            records_deleted++;
        }
    }

    if (records_deleted == 0) {
        printf("Error: No records found with condition %s = %s\n", condition_column_name, condition_value);
        return;
    }


    FILE* f = fopen(fileName, "wb");
    if (!f) {
        perror("Error opening file for writing");
        return;
    }
    if(flock(fileno(f),LOCK_EX) < 0){
    perror("FILE LOCK [CREATE TABLE]");
    exit(-1);
    }

    for(int i = 0;i < target_table->num_columns;i++){
	fprintf(f,"%s",target_table->columns[i]);
	if(i < target_table->num_columns -1)
		fprintf(f,",");
    }

    fprintf(f,"\n");

    for (int i = 0; i < target_table->num_rows; i++) {
        for (int j = 0; j < target_table->num_columns; j++) {
            fprintf(f, "%s", target_table->rows[i].columns[j]);
            if (j < target_table->num_columns - 1) {
                fprintf(f, ",");
            }
        }
        fprintf(f, "\n");
    }

    flock(fileno(f),LOCK_UN);
    fclose(f);
    printf("Records deleted successfully\n");
}




void viewTable(char** parsed) {
    printf("Viewing table %s...\n", parsed[2]);

    char result[20];
    sprintf(result,"%s.dat",parsed[2]);
    DoesTableLoaded = 0;

    for(int i=0;i<current_database.num_tables;i++){
        if(!strcmp(result,current_database.tables[i].name)){
            DoesTableLoaded = 1;
            break;
        }
    }

    if (!DoesTableLoaded) {
        LoadTable(parsed[2]);
        DoesTableLoaded = 1;
    }

    Table* target_table = NULL;

    char* name = (char*)malloc(sizeof(char) * 100);

    sprintf(name, "%s.dat", parsed[2]);
    for (int i = 0; i < current_database.num_tables; i++) {
        if (strcmp(name, current_database.tables[i].name) == 0) {
            target_table = &current_database.tables[i];
            break;
        }
    }

    if (target_table == NULL) {
        printf("Error: Table %s not found\n", parsed[2]);
        return;
    }

    printf("Table: %s\n", target_table->name);

    // Display column names
    printf("Columns: ");
    for (int j = 0; j < target_table->num_columns; j++) {
        printf("%s ", target_table->columns[j]);
    }
    printf("\n");

    printf("Data:\n");
    for (int i = 0; i < target_table->num_rows; i++) {
        for (int j = 0; j < target_table->num_columns; j++) {
            printf("%s ", target_table->rows[i].columns[j]);
        }
        printf("\n");
    }
}

void executeSQL(char** parsed) {
    if (strcmp(parsed[0], "CREATE") == 0 && strcmp(parsed[1], "TABLE") == 0)
        createTable(parsed);
    else if (strcmp(parsed[0], "INSERT") == 0 && strcmp(parsed[1], "INTO") == 0)
        insertInto(parsed);
    else if (strcmp(parsed[0], "VIEW") == 0 && strcmp(parsed[1], "TABLE") == 0)
        viewTable(parsed);
    else if (strcmp(parsed[0], "MODIFY") == 0)
        modifyRecord(parsed);
    else if (strcmp(parsed[0],"DELETE") == 0 && strcmp(parsed[1],"FROM") == 0)
         deleteRecord(parsed);
    else
        printf("Invalid SQL command\n");
}

void parseSpace(char* str, char** parsed) {
    for (int i = 0; i < MAXLIST; i++) {
        parsed[i] = strsep(&str, " ");
        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}

int processString(char* str, char** parsed) {
    parseSpace(str, parsed);

    if (strcmp(parsed[0], "exit") == 0 || strcmp(parsed[0], "quit") == 0 || strcmp(parsed[0], "bye") == 0)
        return 1;
    else if (strcmp(parsed[0], "CREATE") == 0 || strcmp(parsed[0], "INSERT") == 0 || strcmp(parsed[0], "VIEW") == 0 || strcmp(parsed[0],"MODIFY") == 0 || strcmp(parsed[0],"DELETE")==0)
        executeSQL(parsed);
    else
        printf("Command not recognized\n");
    return 0;
}

void LoadDataBase() {
    int number_of_files = 0;
    struct dirent* dent;
    DIR* dir = opendir(databaseDir);
    while ((dent = readdir(dir)) && dent->d_type == 8) {
        strcpy(current_database.tables[number_of_files].name, dent->d_name);
        current_database.num_tables = (number_of_files++);
    }
    current_database.num_tables = 0;
    closedir(dir);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <database_dir> <username>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Create database directory if it doesn't exist
    DIR* dir = opendir(argv[1]);
    if (!dir) {
        mkdir(argv[1], 0777); // Create the database directory
    }
    closedir(dir);

    databaseDir = (char* )malloc(sizeof(char)*strlen(argv[1]));

    databaseDir = strdup(argv[1]); // Store the database directory path
    currentUsername = strdup(argv[2]); // Store the current username

    char* temp = (char*)malloc(sizeof(char)*(strlen(databaseDir)+strlen(currentUsername)+1));
    sprintf(temp,"./%s/%s",databaseDir,currentUsername);

    DIR *d = opendir(temp);
    if(!d){
    mkdir(temp,0777);
    }
    closedir(dir);

    char inputString[MAXCOM], *parsedArgs[MAXLIST];
    //int execFlag = 0;

    system("clear");

    LoadDataBase();
    while (1) {
        if (takeInput(inputString))
            continue;

        if (processString(inputString, parsedArgs))
            break;
    }

    return EXIT_SUCCESS;
}
