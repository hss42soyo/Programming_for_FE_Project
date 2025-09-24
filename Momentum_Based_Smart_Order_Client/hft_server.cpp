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
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <utility>

using namespace std;
using namespace std::chrono;

#define PORT 12345
#define BUFFER_SIZE 1024

struct ClientInfo {
    int socket;
    string name;
    thread clientThread;
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

        {
            lock_guard<mutex> lock(priceMutex);
            priceTimestamps[id] = steady_clock::now();
        }

        {
            lock_guard<mutex> lock(clientsMutex);
            for (auto& client : clients) {
                send(client->socket, message.c_str(), message.size(), 0);
            }
        }

        cout << "📢 Sent price ID " << id << " with value " << price << endl;
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
        cerr << "❌ Failed to receive client name." << endl;
        close(client->socket);
        return;
    }

    client->name = string(buffer);
    cout << "👤 Registered client: " << client->name << endl;

    // Receive orders
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        bytesReceived = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived <= 0) {
            cerr << "❌ Client " << client->name << " disconnected." << endl;
            break;
        }

        int receivedPriceId = atoi(buffer);
        steady_clock::time_point now = steady_clock::now();

        {
            lock_guard<mutex> lock(priceMutex);
            if (priceAlreadyHit.count(receivedPriceId)) {
                // Already hit by another client
                continue;
            }

            if (priceTimestamps.find(receivedPriceId) == priceTimestamps.end()) {
                cerr << "⚠️ Unknown price ID: " << receivedPriceId << endl;
                continue;
            }

            priceAlreadyHit.insert(receivedPriceId);
            auto latency = duration_cast<milliseconds>(now - priceTimestamps[receivedPriceId]).count();
            cout << "🎯 " << client->name << " hit price ID " << receivedPriceId
                 << " after " << latency << " ms" << endl;
        }
    }

    close(client->socket);
}

// Start the server
void startServer() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);  // Localhost

    if (::bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 5) < 0) {
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    cout << "🚀 Server is listening on 127.0.0.1:" << PORT << endl;

    thread priceThread(broadcastPrices);
    priceThread.detach();

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            perror("Accept failed");
            continue;
        }

        cout << "📡 Client connected: " << inet_ntoa(clientAddr.sin_addr) << endl;

        // Create a new client object on the heap
        auto client = make_unique<ClientInfo>();
        client->socket = clientSocket;

        // Store a pointer before moving ownership
        ClientInfo* clientPtr = client.get();

        {
            lock_guard<mutex> lock(clientsMutex);
            clients.push_back(std::move(client));
        }

        // Launch the thread with raw pointer (safe because we hold it in the vector)
        thread t(handleClient, clientPtr);
        t.detach();
        clientPtr->clientThread = std::move(t);  // Optional
    }

    close(serverSocket);
}

int main() {
    startServer();
    return 0;
}
