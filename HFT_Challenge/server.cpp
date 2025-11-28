// server.cpp - Windows version

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <atomic>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <climits>
#include <ctime>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "json.hpp"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;
using namespace std::chrono;
using json = nlohmann::json;

#define PORT 12345
#define BUFFER_SIZE 65536
#define RESULTS_FILE "results.json"
#define MATRIX_SIZE 128
#define MODULO 997

struct ClientInfo {
    SOCKET socket;
    string name;
};

vector<unique_ptr<ClientInfo>> clients;
mutex clientsMutex;

mutex logMutex;
atomic<int> challengeId{1};

struct ChallengeState {
    int id;
    vector<vector<int>> A;
    vector<vector<int>> B;
    steady_clock::time_point startTime;
    unordered_map<string, int> latencies;
};
mutex challengeMutex;
ChallengeState currentChallenge;

vector<vector<int>> generateMatrix(int N) {
    vector<vector<int>> mat(N, vector<int>(N));
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            mat[i][j] = rand() % MODULO;
    return mat;
}

void logChallengeResult(int cid, const unordered_map<string, int>& responses) {
    json entry;
    entry["challenge_id"] = cid;

    string winner = "";
    int bestLatency = INT_MAX;
    for (const auto& [name, latency] : responses) {
        if (latency < bestLatency) {
            bestLatency = latency;
            winner = name;
        }
    }
    entry["winner"] = winner;

    for (const auto& [name, latency] : responses) {
        entry["players"].push_back({{"name", name}, {"latency_ms", latency}});
    }

    json allResults = json::array();
    ifstream in(RESULTS_FILE);
    if (in) {
        try {
            in >> allResults;
        } catch (...) {
            allResults = json::array();
        }
    }
    in.close();

    allResults.push_back(entry);

    ofstream out(RESULTS_FILE);
    out << allResults.dump(2);
    out.close();
}

void handleClient(ClientInfo* client) {
    char buffer[BUFFER_SIZE];

    memset(buffer, 0, BUFFER_SIZE);
    int bytesReceived = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesReceived <= 0) {
        cerr << "❌ Failed to receive client name." << endl;
        closesocket(client->socket);
        return;
    }

    client->name = string(buffer);
    while (!client->name.empty() && (client->name.back() == '\n' || client->name.back() == '\r')) {
        client->name.pop_back();
    }

    cout << "👤 Registered client: " << client->name << endl;

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        bytesReceived = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived <= 0) {
            cerr << "❌ Client " << client->name << " disconnected." << endl;
            break;
        }

        int receivedAnswer = atoi(buffer);
        auto now = steady_clock::now();

        {
            lock_guard<mutex> lock(challengeMutex);
            int latency = (int)duration_cast<milliseconds>(now - currentChallenge.startTime).count();
            currentChallenge.latencies[client->name] = latency;

            cout << "📤 " << client->name << " answered " << receivedAnswer
                 << " in " << latency << " ms (challenge " << currentChallenge.id << ")" << endl;
        }
    }

    closesocket(client->socket);
}

void broadcastChallengeLoop() {
    while (true) {
        int cid = challengeId++;
        vector<vector<int>> A = generateMatrix(MATRIX_SIZE);
        vector<vector<int>> B = generateMatrix(MATRIX_SIZE);

        {
            lock_guard<mutex> lock(challengeMutex);
            currentChallenge = {cid, A, B, steady_clock::now(), {}};
        }

        // 序列化 challenge
        stringstream ss;
        ss << cid << "\n";
        ss << MATRIX_SIZE << "\n";
        for (const auto& row : A)
            for (int val : row)
                ss << val << " ";
        ss << "\n";
        for (const auto& row : B)
            for (int val : row)
                ss << val << " ";
        ss << "\n";

        string payload = ss.str();

        {
            lock_guard<mutex> lock(clientsMutex);
            for (auto& client : clients) {
                if (client->socket != INVALID_SOCKET) {
                    int sent = send(client->socket, payload.c_str(), (int)payload.size(), 0);
                    if (sent == SOCKET_ERROR) {
                        cerr << "❌ Failed to send to client, error: " << WSAGetLastError() << endl;
                    }
                }
            }
        }

        cout << "📢 Broadcasted matrix challenge " << cid << " to all clients" << endl;

        this_thread::sleep_for(seconds(10));

        unordered_map<string, int> snapshot;
        {
            lock_guard<mutex> lock(challengeMutex);
            snapshot = currentChallenge.latencies;
        }

        {
            lock_guard<mutex> lock(logMutex);
            logChallengeResult(cid, snapshot);
        }
    }
}

void startServer() {
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0) {
        cerr << "WSAStartup failed: " << wsaResult << endl;
        return;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed, error: " << WSAGetLastError() << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
               (const char*)&opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (::bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed, error: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        cerr << "Listen failed, error: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    cout << "🚀 Server is listening on 127.0.0.1:" << PORT << endl;

    thread broadcaster(broadcastChallengeLoop);
    broadcaster.detach();

    while (true) {
        sockaddr_in clientAddr{};
        int clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Accept failed, error: " << WSAGetLastError() << endl;
            continue;
        }

        char ipStr[INET_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, sizeof(ipStr));
        cout << "📡 Client connected: " << ipStr << endl;

        auto client = make_unique<ClientInfo>();
        client->socket = clientSocket;
        ClientInfo* clientPtr = client.get();

        {
            lock_guard<mutex> lock(clientsMutex);
            clients.push_back(std::move(client));
        }

        thread t(handleClient, clientPtr);
        t.detach();
    }

    closesocket(serverSocket);
    WSACleanup();
}

int main() {
    srand((unsigned int)time(nullptr));
    startServer();
    return 0;
}
