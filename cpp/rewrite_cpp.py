import re

def rewrite_database_cpp():
    with open('d:/Unbound/pokemon/cpp/server/database/database.cpp', 'r', encoding='utf-8') as f:
        content = f.read()

    # Add logger include
    if '#include "utils/logger.h"' not in content:
        content = content.replace('#include "database/database.h"', '#include "database/database.h"\n#include "utils/logger.h"')

    # Replace constructor
    old_ctor = '''DatabasePool::DatabasePool(const std::string& connection_string, size_t pool_size)
    : connection_string_(connection_string) {
  for (size_t i = 0; i < pool_size; ++i) {
    try {
      auto conn = std::make_unique<pqxx::connection>(connection_string_);
      PrepareStatements(*conn);
      pool_.push_back(std::move(conn));
    } catch (const std::exception& e) {
      std::cerr << "Failed to connect to database: " << e.what() << std::endl;
      throw;
    }
  }
}'''

    new_ctor = '''DatabasePool::DatabasePool(const std::string& connection_string, size_t pool_size)
    : connection_string_(connection_string) {
  try {
    Logger::Info("Connecting to PostgreSQL...");
    pqxx::connection init_conn(connection_string_);
    Logger::Info("Connected.");

    InitializeSchema(init_conn);

    Logger::Info("Registering prepared statements...");
    for (size_t i = 0; i < pool_size; ++i) {
      auto conn = std::make_unique<pqxx::connection>(connection_string_);
      PrepareStatements(*conn);
      pool_.push_back(std::move(conn));
    }
    Logger::Info("Database initialization complete.");
  } catch (const std::exception& e) {
    // The exception is caught and logged in main.cpp, but the user expects the exact throw behavior.
    throw;
  }
}'''
    
    content = content.replace(old_ctor, new_ctor)

    # Replace InitializeSchema
    # Need to match from void DatabasePool::InitializeSchema() { to return true; \n  });\n}
    old_schema_regex = r"void DatabasePool::InitializeSchema\(\) \{[\s\S]*?return true;\n  \}\);\n\}"
    
    new_schema = '''void DatabasePool::InitializeSchema(pqxx::connection& conn) {
  Logger::Info("Creating database schema...");
  pqxx::work txn(conn);
  try {
    txn.exec(R"(
      CREATE TABLE IF NOT EXISTS schema_version (
        version INTEGER PRIMARY KEY,
        applied_at BIGINT NOT NULL
      );
    )");

    txn.exec(R"(
      CREATE TABLE IF NOT EXISTS accounts (
        id BIGSERIAL PRIMARY KEY,
        username VARCHAR(255) UNIQUE NOT NULL,
        password_hash VARCHAR(255) NOT NULL,
        created_at BIGINT NOT NULL
      );
    )");
    Logger::Info("Created table: accounts");
    
    txn.exec(R"(
      CREATE TABLE IF NOT EXISTS characters (
        id BIGSERIAL PRIMARY KEY,
        account_id BIGINT NOT NULL REFERENCES accounts(id) ON DELETE CASCADE,
        name VARCHAR(255) NOT NULL,
        appearance VARCHAR(255) NOT NULL,
        play_time_seconds BIGINT NOT NULL DEFAULT 0,
        created_at BIGINT NOT NULL,
        last_login BIGINT NOT NULL,
        map_id INTEGER NOT NULL DEFAULT 0,
        x REAL NOT NULL DEFAULT 0.0,
        y REAL NOT NULL DEFAULT 0.0,
        direction SMALLINT NOT NULL DEFAULT 0,
        money BIGINT NOT NULL DEFAULT 0
      );
    )");
    Logger::Info("Created table: characters");

    txn.exec(R"(
      CREATE TABLE IF NOT EXISTS inventory_blobs (
        character_id BIGINT PRIMARY KEY REFERENCES characters(id) ON DELETE CASCADE,
        blob_data BYTEA NOT NULL
      );
    )");
    Logger::Info("Created table: inventory_blobs");

    txn.exec(R"(
      CREATE TABLE IF NOT EXISTS party_blobs (
        character_id BIGINT NOT NULL REFERENCES characters(id) ON DELETE CASCADE,
        slot_index SMALLINT NOT NULL,
        blob_data BYTEA NOT NULL,
        PRIMARY KEY (character_id, slot_index)
      );
    )");
    Logger::Info("Created table: party_blobs");

    txn.exec(R"(
      CREATE TABLE IF NOT EXISTS pc_blobs (
        character_id BIGINT PRIMARY KEY REFERENCES characters(id) ON DELETE CASCADE,
        blob_data BYTEA NOT NULL
      );
    )");
    Logger::Info("Created table: pc_blobs");

    txn.exec(R"(
      CREATE TABLE IF NOT EXISTS story_blobs (
        character_id BIGINT PRIMARY KEY REFERENCES characters(id) ON DELETE CASCADE,
        flags_blob BYTEA NOT NULL,
        badges_blob BYTEA NOT NULL,
        quests_blob BYTEA NOT NULL
      );
    )");
    Logger::Info("Created table: story_blobs");

    txn.exec(R"(
      INSERT INTO schema_version (version, applied_at)
      SELECT 1, EXTRACT(EPOCH FROM NOW())::BIGINT
      WHERE NOT EXISTS (SELECT 1 FROM schema_version);
    )");

    txn.commit();
  } catch (const std::exception& e) {
    txn.abort();
    throw;
  }
}'''

    content = re.sub(old_schema_regex, new_schema, content)

    with open('d:/Unbound/pokemon/cpp/server/database/database.cpp', 'w', encoding='utf-8') as f:
        f.write(content)

rewrite_database_cpp()
print("database.cpp updated")
