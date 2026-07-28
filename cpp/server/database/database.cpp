#include "database/database.h"
#include "utils/logger.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace unboundmp::server {

DatabasePool::DatabasePool(const std::string& connection_string, size_t pool_size)
    : connection_string_(connection_string) {
  try {
    Logger::Info("Connecting to PostgreSQL...");
    pqxx::connection init_conn(connection_string_.c_str());
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
}

DatabasePool::~DatabasePool() {
  std::lock_guard<std::mutex> lock(pool_mutex_);
  pool_.clear();
}

void DatabasePool::InitializeSchema(pqxx::connection& conn) {
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
        money BIGINT NOT NULL DEFAULT 0,
          save_state_blob BYTEA
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
}

std::unique_ptr<pqxx::connection> DatabasePool::AcquireConnection() {
  std::unique_lock<std::mutex> lock(pool_mutex_);
  while (pool_.empty()) {
    lock.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    lock.lock();
  }
  
  auto conn = std::move(pool_.back());
  pool_.pop_back();
  return conn;
}

void DatabasePool::ReleaseConnection(std::unique_ptr<pqxx::connection> conn) {
  std::lock_guard<std::mutex> lock(pool_mutex_);
  pool_.push_back(std::move(conn));
}

void DatabasePool::PrepareStatements(pqxx::connection& conn) {
  conn.prepare("create_account", "INSERT INTO accounts (username, password_hash, created_at) VALUES ($1, $2, $3) RETURNING id");
  conn.prepare("get_account_by_username", "SELECT id, username, password_hash, created_at FROM accounts WHERE username = $1");
  conn.prepare("get_account_by_id", "SELECT id, username, password_hash, created_at FROM accounts WHERE id = $1");
  conn.prepare("delete_account", "DELETE FROM accounts WHERE id = $1");
  conn.prepare("update_password", "UPDATE accounts SET password_hash = $1 WHERE id = $2");
  
  conn.prepare("create_character", "INSERT INTO characters (account_id, name, appearance, play_time_seconds, created_at, last_login, map_id, x, y, direction, money) VALUES ($1, $2, $3, $4, $5, $6, 0, 0, 0, 0, 0) RETURNING id");
  conn.prepare("get_character_by_account", "SELECT id, account_id, name, appearance, play_time_seconds, created_at, last_login, map_id, x, y, direction, money FROM characters WHERE account_id = $1");
  conn.prepare("get_character_by_id", "SELECT id, account_id, name, appearance, play_time_seconds, created_at, last_login, map_id, x, y, direction, money FROM characters WHERE id = $1");
  conn.prepare("check_character_name", "SELECT 1 FROM characters WHERE name = $1");
  conn.prepare("check_character_exists", "SELECT 1 FROM characters WHERE id = $1");
  conn.prepare("rename_character", "UPDATE characters SET name = $1 WHERE id = $2");
  conn.prepare("update_character", "UPDATE characters SET name = $1, appearance = $2, play_time_seconds = $3, last_login = $4, map_id = $5, x = $6, y = $7, direction = $8, money = $9 WHERE id = $10");
  conn.prepare("delete_character", "DELETE FROM characters WHERE id = $1");
  conn.prepare("delete_character_safe", "DELETE FROM characters WHERE id = $1 AND account_id = $2");

  // Stage 8: Blob operations
  conn.prepare("upsert_inventory_blob", "INSERT INTO inventory_blobs (character_id, blob_data) VALUES ($1, $2) ON CONFLICT (character_id) DO UPDATE SET blob_data = EXCLUDED.blob_data");
  conn.prepare("get_inventory_blob", "SELECT blob_data FROM inventory_blobs WHERE character_id = $1");

  conn.prepare("upsert_party_blob", "INSERT INTO party_blobs (character_id, slot_index, blob_data) VALUES ($1, $2, $3) ON CONFLICT (character_id, slot_index) DO UPDATE SET blob_data = EXCLUDED.blob_data");
  conn.prepare("get_party_blobs", "SELECT slot_index, blob_data FROM party_blobs WHERE character_id = $1 ORDER BY slot_index ASC");

  conn.prepare("upsert_pc_blob", "INSERT INTO pc_blobs (character_id, blob_data) VALUES ($1, $2) ON CONFLICT (character_id) DO UPDATE SET blob_data = EXCLUDED.blob_data");
  conn.prepare("get_pc_blob", "SELECT blob_data FROM pc_blobs WHERE character_id = $1");

  conn.prepare("upsert_story_blobs", "INSERT INTO story_blobs (character_id, flags_blob, badges_blob, quests_blob) VALUES ($1, $2, $3, $4) ON CONFLICT (character_id) DO UPDATE SET flags_blob = EXCLUDED.flags_blob, badges_blob = EXCLUDED.badges_blob, quests_blob = EXCLUDED.quests_blob");
  conn.prepare("get_story_blobs", "SELECT flags_blob, badges_blob, quests_blob FROM story_blobs WHERE character_id = $1");
}

}  // namespace unboundmp::server
