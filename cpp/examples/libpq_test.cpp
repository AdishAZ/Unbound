#include <iostream>
#include <libpq-fe.h>
#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    std::cout << "Connecting via libpq directly..." << std::endl;
    PGconn* conn = PQconnectdb("host=127.0.0.1 port=5432 dbname=unboundmp user=postgres password=12345678");
    if (PQstatus(conn) == CONNECTION_BAD) {
        std::cerr << "Connection to database failed: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        return 1;
    }
    std::cout << "Connected to PostgreSQL version: " << PQserverVersion(conn) << std::endl;
    PQfinish(conn);
    return 0;
}
