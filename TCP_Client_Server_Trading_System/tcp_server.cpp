#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <sstream>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

struct Order {
    std::string type;     // BUY or SELL
    std::string symbol;   // e.g., AAPL
    int quantity;
    double price;
};

std::vector<Order> orderBook;
std::mutex bookMutex;

bool parseOrder(const std::string& msg, Order& order) {
    std::istringstream iss(msg);
    if (!(iss >> order.type >> order.symbol >> order.quantity >> order.price))
        return false;

    if ((order.type != "BUY" && order.type != "SELL") || order.quantity <= 0 || order.price <= 0)
        return false;

    return true;
}

void handleClient(SOCKET clientSocket) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0)
            break;

        buffer[bytesReceived] = '\0';
        std::string msg(buffer);

        Order order;
        std::string response;

        if (parseOrder(msg, order)) {
            {
                std::lock_guard<std::mutex> lock(bookMutex);
                orderBook.push_back(order);
            }
            std::cout << "Received: " << order.type << " " << order.symbol
                      << " " << order.quantity << " @ " << order.price << std::endl;

            response = "CONFIRMED: " + order.type + " " + order.symbol +
                       " " + std::to_string(order.quantity) + " @ " + std::to_string(order.price);
        } else {
            response = "ERROR: Invalid format";
        }

        send(clientSocket, response.c_str(), static_cast<int>(response.size()), 0);
    }

    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);

    std::cout << "Server started on port 54000..." << std::endl;

    while (true) {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

        std::thread t(handleClient, clientSocket);
        t.detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
