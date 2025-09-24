// client_windows.cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <string>

#ifdef _WIN32
  #include <winsock2.h>      // Winsock2
  #include <ws2tcpip.h>      // inet_pton 等
  #pragma comment(lib, "Ws2_32.lib")
  using socklen_t = int;
  #define CLOSESOCKET closesocket
#else
  #include <unistd.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #define SOCKET int
  #define INVALID_SOCKET (-1)
  #define SOCKET_ERROR  (-1)
  #define CLOSESOCKET close
#endif

using namespace std;

#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

// 收价格→随机延迟→回发订单(价格ID)
void receiveAndRespond(SOCKET socketFd, const string& name) {
    char buffer[BUFFER_SIZE];

    // 发送客户端名
    send(socketFd, name.c_str(), (int)name.size(), 0);

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(socketFd, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived <= 0) {
            cerr << "Server closed connection or error occurred." << endl;
            break;
        }

        string data(buffer);
        size_t commaPos = data.find(',');
        if (commaPos == string::npos) {
            cerr << "Invalid price format received: " << data << endl;
            continue;
        }

        int   priceId = stoi(data.substr(0, commaPos));
        float price   = stof(data.substr(commaPos + 1));

        cout << "📥 Received price ID: " << priceId << ", Value: " << price << endl;

        // 模拟反应延迟
        this_thread::sleep_for(chrono::milliseconds(100 + rand() % 300));

        // 回发订单(仅价格ID)
        string order = to_string(priceId);
        send(socketFd, order.c_str(), (int)order.size(), 0);

        cout << "📤 Sent order for price ID: " << priceId << endl;
    }

    CLOSESOCKET(socketFd);
}

int main() {
    srand((unsigned)time(nullptr));

#ifdef _WIN32
    // 初始化 Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }
#endif

    string name;
    cout << "Enter your client name: ";
    getline(cin, name);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation failed!" << endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port   = htons(SERVER_PORT);
    // 127.0.0.1 → 二进制
    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) != 1) {
        cerr << "inet_pton failed!" << endl;
        CLOSESOCKET(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    if (connect(sock, (sockaddr*)&serverAddr, (socklen_t)sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Connection to server failed!" << endl;
        CLOSESOCKET(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    cout << "✅ Connected to server at " << SERVER_IP << ":" << SERVER_PORT << endl;
    receiveAndRespond(sock, name);

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
