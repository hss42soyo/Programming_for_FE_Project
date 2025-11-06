#include <iostream>
#include <string>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET clientSocket;
    sockaddr_in serverAddr;

    while (true) {
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(54000);
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        std::cout << "Connecting to server..." << std::endl;
        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cout << "Connection failed. Retrying in 3 seconds..." << std::endl;
            closesocket(clientSocket);
            Sleep(3000);
            continue;
        }
        std::cout << "Connected to server!" << std::endl;
        break;
    }

    while (true) {
        std::string input;
        std::cout << "\nEnter order (e.g., BUY AAPL 100 150.25), or 'exit': ";
        std::getline(std::cin, input);
        if (input == "exit")
            break;

        send(clientSocket, input.c_str(), static_cast<int>(input.size()), 0);

        char buffer[1024];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0)
            break;

        buffer[bytesReceived] = '\0';
        std::cout << "Server Response: " << buffer << std::endl;
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
