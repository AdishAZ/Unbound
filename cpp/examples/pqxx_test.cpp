#include <iostream>
#include <pqxx/pqxx>
#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    std::cout << "Connecting..." << std::endl;
    try {
        pqxx::connection c("host=127.0.0.1 port=5432 dbname=unboundmp user=postgres password=12345678");
        std::cout << "Connected to PostgreSQL version: " << c.server_version() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
