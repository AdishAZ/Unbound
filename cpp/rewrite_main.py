import re

def rewrite_main():
    with open('d:/Unbound/pokemon/cpp/server/main.cpp', 'r', encoding='utf-8') as f:
        content = f.read()
    
    old_try = '''  std::shared_ptr<DatabasePool> db_pool;
  try {
    db_pool = std::make_shared<DatabasePool>(config.GetConnectionString(), 4);
    db_pool->InitializeSchema();
    Logger::Info("Database schema initialized.");
  } catch (const std::exception& e) {
    Logger::Error(std::string("Database initialization failed: ") + e.what());
    // In production we would exit, but we might want to continue in tests
  }'''
    
    new_try = '''  std::shared_ptr<DatabasePool> db_pool;
  try {
    db_pool = std::make_shared<DatabasePool>(config.GetConnectionString(), 4);
  } catch (const std::exception& e) {
    Logger::Error(std::string("Database initialization failed: ") + e.what());
    return 1;
  }'''

    content = content.replace(old_try, new_try)
    
    # Also remove "Connecting to Database..." if it's there, as we do it in pool now
    content = content.replace('  Logger::Info("Connecting to Database...");\n', '')
    
    with open('d:/Unbound/pokemon/cpp/server/main.cpp', 'w', encoding='utf-8') as f:
        f.write(content)
        
rewrite_main()
print("main.cpp updated")
