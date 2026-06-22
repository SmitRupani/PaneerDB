#include "Engine.h"
#include "statements/Statement.h"
#include "statements/UseDatabaseStatement.h"
#include <stdexcept>

Engine::Engine() : active(false), activeDbName("") {}

void Engine::useDatabase(const std::string &name) {
  activeDbName = name;
  active = true;
}

void Engine::execute(Statement* statement) {
  if (!statement) return;

  switch (statement->getType()) {
    case Statement::StatementType::USE_DATABASE: {
      auto useDbStmt = static_cast<UseDatabaseStatement*>(statement);
      useDatabase(useDbStmt->dbName);
      break;
    }
    default:
      // Other statement types will be handled here later
      break;
  }
}

bool Engine::isDbActive() const { return active; }

std::string Engine::getActiveDatabase() const {
  if (!active) {
    throw std::runtime_error("No active database.");
  }
  return activeDbName;
}
