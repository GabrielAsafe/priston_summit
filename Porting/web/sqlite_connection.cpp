#include <iostream>
#include <string>
#include <sqlite3.h>

// Protótipos
int openDB(sqlite3** DB); // recebe o valor do ponteiro, não o ponteiro
int createTable(sqlite3* DB);//recebe o ponteiro 
int insertInTable(sqlite3* DB);
int deleteFromTable(sqlite3* DB);
int selectData(sqlite3* DB);
void error_message(int rc, const std::string& message, const char* detail);

static int callback(
    void* data,
    int argc,
    char** argv,
    char** azColName
);


// rc -> return code

int main()
{
    sqlite3* DB = nullptr; // cria um ponteiro para o objetosqlite3

    // passar o endereço do ponteiro faz ele ser um ponteiro para ponteiro (endereço que aponta para o ponteiro que aponta para o objeto sqlite3)
    //peis precisamos alterar o objeto
    if (openDB(&DB) != SQLITE_OK) {
        return 1;
    }
    //aqui eu passo o ponteiro pois só precisamos executar algo, não criar
    if (createTable(DB) != SQLITE_OK) {
        sqlite3_close(DB);
        return 1;
    }

    if (insertInTable(DB) != SQLITE_OK) {
        sqlite3_close(DB);
        return 1;
    }


    if (selectData(DB) != SQLITE_OK) {
        sqlite3_close(DB);
        return 1;
    }

    if (deleteFromTable(DB) != SQLITE_OK) {
        sqlite3_close(DB);
        return 1;
    }


    sqlite3_close(DB);

    return 0;
}

//como DB é um ponteiro e eu vou mandar o endereço dele, minha função precisa receber o ponteiro para um ponteiro
int openDB(sqlite3** DB)
{
    int rc = sqlite3_open("example.db", DB);
    if (rc != SQLITE_OK) {
        const char* detail = (*DB != nullptr) ? sqlite3_errmsg(*DB) : sqlite3_errstr(rc);
        error_message(rc, "Error opening DB: ", detail);

        if (*DB != nullptr) {
            sqlite3_close(*DB);
            *DB = nullptr;
        }

        return rc;
    }

    std::cout << "Opened Database Successfully!\n";

    return SQLITE_OK;
}

//pega o bjeto DB
int createTable(sqlite3* DB)
{
    std::string sql =
        "CREATE TABLE IF NOT EXISTS PERSON("
        "ID INTEGER PRIMARY KEY NOT NULL, "
        "NAME TEXT NOT NULL, "
        "SURNAME TEXT NOT NULL, "
        "AGE INTEGER NOT NULL, "
        "ADDRESS TEXT, "
        "SALARY REAL"
        ");";

    char* messageError = nullptr;

    int rc = sqlite3_exec(
        DB,
        sql.c_str(),
        nullptr,
        nullptr,
        &messageError
    );


    if (rc != SQLITE_OK) {
        error_message(rc, "Error creating table: ",
                      messageError ? messageError : sqlite3_errmsg(DB));
        sqlite3_free(messageError);
        return rc;
    }

    std::cout << "Table created successfully!\n";

    return SQLITE_OK;
}


int insertInTable(sqlite3* DB)
{
    std::string sql =
        "INSERT OR IGNORE INTO PERSON "
        "VALUES(1, 'STEVE', 'GATES', 30, 'PALO ALTO', 1000.0);"

        "INSERT OR IGNORE INTO PERSON "
        "VALUES(2, 'BILL', 'ALLEN', 20, 'SEATTLE', 300.22);"

        "INSERT OR IGNORE INTO PERSON "
        "VALUES(3, 'PAUL', 'JOBS', 24, 'SEATTLE', 9900.0);";

    char* messageError = nullptr;

    int rc = sqlite3_exec(
        DB,
        sql.c_str(),
        nullptr,
        nullptr,
        &messageError
    );



     if (rc != SQLITE_OK) {
        error_message(rc, "Error inserting data: ",
                      messageError ? messageError : sqlite3_errmsg(DB));
        sqlite3_free(messageError);
        return rc;
    }

    std::cout << "Records created successfully!\n";

    return SQLITE_OK;
}


int selectData(sqlite3* DB)
{
    std::string sql = "SELECT * FROM PERSON;";

    std::string data = "PERSON";

    int rc = sqlite3_exec(
        DB,
        sql.c_str(),
        callback,
        static_cast<void*>(data.data()),
        nullptr
    );

    if (rc != SQLITE_OK) {
        error_message(rc, "Error selecting record: ", sqlite3_errmsg(DB));
        return rc;
    }

    std::cout << "SELECT completed successfully!\n";

    return SQLITE_OK;
}


int deleteFromTable(sqlite3* DB)
{
    std::string sql =
        "DELETE FROM PERSON WHERE ID = 2;";

    char* messageError = nullptr;

    int rc = sqlite3_exec(
        DB,
        sql.c_str(),
        nullptr,
        nullptr,
        &messageError
    );


    if (rc != SQLITE_OK) {
        error_message(rc, "Error deleting record: ",
                      messageError ? messageError : sqlite3_errmsg(DB));
        sqlite3_free(messageError);
        return rc;
    }

    

    std::cout << "Record deleted successfully!\n";

    return SQLITE_OK;
}


static int callback(
    void* data,
    int argc,
    char** argv,
    char** azColName
)
{
    const char* label =
        static_cast<const char*>(data);

    std::cout << label << ":\n";

    for (int i = 0; i < argc; i++) {

        std::cout
            << azColName[i]
            << " = "
            << (argv[i] ? argv[i] : "NULL")
            << '\n';
    }

    std::cout << '\n';

    return 0;
}

void error_message(int rc, const std::string& message, const char* detail)
{
    std::cerr
        << message
        << (detail ? detail : "unknown SQLite error")
        << " (rc=" << rc << ")\n";
}
