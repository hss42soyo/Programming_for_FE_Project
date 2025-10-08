// chat_server_cross.cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <atomic>
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "Ws2_32.lib")
  using socklen_t = int;
  #define CLOSESOCKET closesocket
  #define GET_LAST_ERR() WSAGetLastError()
#else
  #include <unistd.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #define SOCKET int
  #define INVALID_SOCKET (-1)
  #define SOCKET_ERROR  (-1)
  #define CLOSESOCKET close
  #define GET_LAST_ERR() errno
#endif

using namespace std;
using namespace std::chrono;

#define PORT 12345
#define BUFFER_SIZE 1024

struct ClientInfo {
    SOCKET socket;
    string name;
};

vector<unique_ptr<ClientInfo>> clients;
mutex clientsMutex;

unordered_map<int, steady_clock::time_point> priceTimestamps;
unordered_set<int> priceAlreadyHit;
mutex priceMutex;

atomic<int> priceId{0};

// Send new price every 5 seconds
void broadcastPrices() {
    while (true) {
        int id = priceId++;
        float price = 100.0f + (rand() % 1000) / 10.0f;
        string message = to_string(id) + "," + to_string(price);

        { lock_guard<mutex> lock(priceMutex);
          priceTimestamps[id] = steady_clock::now(); }

        { lock_guard<mutex> lock(clientsMutex);
          for (auto& c : clients) {
              (void)send(c->socket, message.c_str(), (int)message.size(), 0);
          } }

        cout << "📢 Sent price ID " << id << " value " << price << endl;
        this_thread::sleep_for(chrono::seconds(5));
    }
}

// Handle a client connection
void handleClient(ClientInfo* client) {
    char buffer[BUFFER_SIZE];

    // Receive client name
    memset(buffer, 0, BUFFER_SIZE);
    int bytesReceived = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesReceived <= 0) {
        cerr << "❌ Failed to receive client name. err=" << GET_LAST_ERR() << endl;
        CLOSESOCKET(client->socket);
        return;
    }
    client->name = string(buffer);
    cout << "👤 Registered client: " << client->name << endl;

    // Receive orders
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        bytesReceived = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived <= 0) {
            cerr << "❌ Client " << client->name << " disconnected. err=" << GET_LAST_ERR() << endl;
            break;
        }

        int receivedPriceId = atoi(buffer);
        auto now = steady_clock::now();

        lock_guard<mutex> lock(priceMutex);
        if (priceAlreadyHit.count(receivedPriceId)) {
            // Already hit by another client
            continue;
        }
        auto it = priceTimestamps.find(receivedPriceId);
        if (it == priceTimestamps.end()) {
            cerr << "⚠️ Unknown price ID: " << receivedPriceId << endl;
            continue;
        }
        priceAlreadyHit.insert(receivedPriceId);
        auto latency = duration_cast<milliseconds>(now - it->second).count();
        cout << "🎯 " << client->name << " hit " << receivedPriceId
             << " after " << latency << " ms" << endl;
    }

    CLOSESOCKET(client->socket);
}

// Start the server
void startServer() {
#ifdef _WIN32
    // Windows initialize Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        cerr << "WSAStartup failed: " << GET_LAST_ERR() << endl;
        exit(EXIT_FAILURE);
    }
#endif

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed: " << GET_LAST_ERR() << endl;
#ifdef _WIN32
        WSACleanup();
#endif
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
                   (const char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        cerr << "setsockopt failed: " << GET_LAST_ERR() << endl;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    // 127.0.0.1
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (::bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed: " << GET_LAST_ERR() << endl;
        CLOSESOCKET(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        cerr << "Listen failed: " << GET_LAST_ERR() << endl;
        CLOSESOCKET(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        exit(EXIT_FAILURE);
    }

    cout << "🚀 Server listening on 127.0.0.1:" << PORT << endl;

    thread(broadcastPrices).detach();

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = (socklen_t)sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Accept failed: " << GET_LAST_ERR() << endl;
            continue;
        }

        char ip[INET_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));
        cout << "📡 Client connected: " << ip << endl;

        auto cli = make_unique<ClientInfo>();
        cli->socket = clientSocket;
        ClientInfo* ptr = cli.get();
        {
            lock_guard<mutex> lock(clientsMutex);
            clients.push_back(std::move(cli));
        }
        thread(handleClient, ptr).detach();
    }

    CLOSESOCKET(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
}

int main() {
    srand((unsigned)time(nullptr));
    startServer();
    return 0;
}
