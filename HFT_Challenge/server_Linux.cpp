#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <atomic>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include "json.hpp"

using namespace std;
using namespace std::chrono;
using json = nlohmann::json;

#define PORT 12345
#define BUFFER_SIZE 65536
#define RESULTS_FILE "/tmp/results.json"
#define MATRIX_SIZE 128
#define MODULO 997

struct ClientInfo {
    int socket;
    string name;
    thread clientThread;
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

    json allResults;
    ifstream in(RESULTS_FILE);
    if (in) {
        try { in >> allResults; } catch (...) { allResults = json::array(); }
    }
    in.close();

    allResults.push_back(entry);

    ofstream out(RESULTS_FILE);
    out << allResults.dump(2);
}

void handleClient(ClientInfo* client) {
    char buffer[BUFFER_SIZE];

    // Receive group name
    memset(buffer, 0, BUFFER_SIZE);
    int bytesReceived = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesReceived <= 0) {
        cerr << "❌ Failed to receive client name." << endl;
        close(client->socket);
        return;
    }

    client->name = string(buffer);
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

        lock_guard<mutex> lock(challengeMutex);
        int latency = duration_cast<milliseconds>(now - currentChallenge.startTime).count();
        currentChallenge.latencies[client->name] = latency;

        cout << "📤 " << client->name << " answered " << receivedAnswer
             << " in " << latency << " ms (challenge " << currentChallenge.id << ")" << endl;
    }

    close(client->socket);
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

        // Serialize challenge
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
                send(client->socket, payload.c_str(), payload.size(), 0);
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
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (::bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 10) < 0) {
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    cout << "🚀 Server is listening on 127.0.0.1:" << PORT << endl;

    thread broadcaster(broadcastChallengeLoop);
    broadcaster.detach();

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            perror("Accept failed");
            continue;
        }

        cout << "📡 Client connected: " << inet_ntoa(clientAddr.sin_addr) << endl;

        auto client = make_unique<ClientInfo>();
        client->socket = clientSocket;
        ClientInfo* clientPtr = client.get();

        {
            lock_guard<mutex> lock(clientsMutex);
            clients.push_back(std::move(client));
        }

        thread t(handleClient, clientPtr);
        t.detach();
        clientPtr->clientThread = std::move(t);
    }

    close(serverSocket);
}

int main() {
    srand(time(nullptr));
    startServer();
    return 0;
}
