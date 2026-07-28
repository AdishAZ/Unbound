import re

def rewrite_database_h():
    with open('d:/Unbound/pokemon/cpp/server/database/database.h', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Remove void InitializeSchema(); from public
    content = content.replace('  // Initialize schema (creates tables if they do not exist)\n  void InitializeSchema();\n', '')
    
    # Add to private
    private_block = ''' private:
  void InitializeSchema(pqxx::connection& conn);
  std::unique_ptr<pqxx::connection> AcquireConnection();'''
    
    content = content.replace(' private:\n  std::unique_ptr<pqxx::connection> AcquireConnection();', private_block)
    
    with open('d:/Unbound/pokemon/cpp/server/database/database.h', 'w', encoding='utf-8') as f:
        f.write(content)

rewrite_database_h()
print("database.h updated")
