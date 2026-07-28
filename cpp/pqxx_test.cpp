#include <iostream>
#include <pqxx/pqxx>

int main() {
    std::cout << "Connecting..." << std::endl;
    try {
        pqxx::connection c("host=127.0.0.1 port=5432 dbname=unboundmp user=postgres password=12345678");
        std::cout << "Connected to PostgreSQL version: " << c.server_version() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
