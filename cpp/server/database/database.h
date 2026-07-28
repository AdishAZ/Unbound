#pragma once

#include <pqxx/pqxx>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <functional>

namespace unboundmp::server {

class DatabasePool {
 public:
  DatabasePool(const std::string& connection_string, size_t pool_size);
  ~DatabasePool();
  
  DatabasePool(const DatabasePool&) = delete;
  DatabasePool& operator=(const DatabasePool&) = delete;


  // Execute a transaction using a connection from the pool
  template <typename Func>
  auto ExecuteTransaction(Func&& func) {
    auto conn = AcquireConnection();
    try {
      pqxx::work txn(*conn);
      auto result = func(txn);
      txn.commit();
      ReleaseConnection(std::move(conn));
      return result;
    } catch (...) {
      ReleaseConnection(std::move(conn));
      throw;
    }
  }

 private:
  void InitializeSchema(pqxx::connection& conn);
  std::unique_ptr<pqxx::connection> AcquireConnection();
  void ReleaseConnection(std::unique_ptr<pqxx::connection> conn);
  
  void PrepareStatements(pqxx::connection& conn);

  std::string connection_string_;
  
  std::mutex pool_mutex_;
  std::vector<std::unique_ptr<pqxx::connection>> pool_;
};

}  // namespace unboundmp::server
