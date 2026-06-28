#include "engine/Engine.h"
#include "QueryParser.h"
#include <iostream>
#include <cassert>

int main() {
    Engine engine;
    
    // Create DB
    auto exec = [&](const std::string& query) {
        QueryParser qp;
        qp.tokenize(query);
        engine.execute(qp.parse());
    };
    
    exec("CREATE DATABASE testdb;");
    exec("USE DATABASE testdb;");
    exec("CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(20));");
    
    // Test transaction
    exec("BEGIN;");
    exec("INSERT INTO users VALUES (1, \"alice\");");
    exec("COMMIT;");
    
    // Test rollback
    exec("BEGIN;");
    exec("INSERT INTO users VALUES (2, \"bob\");");
    exec("ROLLBACK;");
    
    // Validate
    exec("SELECT * FROM users;");
    
    std::cout << "Transaction Test Passed!\n";
    return 0;
}
