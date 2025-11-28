// client.cpp - Windows HFT Matrix Challenge client

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <cctype>
#include <chrono>

#pragma comment(lib, "Ws2_32.lib")

// Configuration
const char* SERVER_IP   = "127.0.0.1";
const int   SERVER_PORT = 12345;
const int   MODULO      = 997;
const std::string MY_GROUP_NAME = "Group1";


// Read int from TCP stream
class IntStreamReader {
public:
    explicit IntStreamReader(SOCKET s) : sock(s), pos(0) {}

    // Read an int from the stream, return true on success, false on failure (disconnect/error)
    bool readInt(int& value) {
        while (true) {
            //Skip whitespace
            skipSpaces();

            // Buffer used up
            if (pos >= buffer.size()) {
                if (!fillBuffer()) {
                    return false; // Connection closed
                }
                continue;
            }

            // Handle possible sign
            bool neg = false;
            if (buffer[pos] == '+' || buffer[pos] == '-') {
                neg = (buffer[pos] == '-');
                ++pos;

                if (pos >= buffer.size()) {
                    if (!fillBuffer()) return false;
                }
            }

            // Ensure Digit
            if (pos >= buffer.size()) {
                if (!fillBuffer()) return false;
            }
            if (!std::isdigit(static_cast<unsigned char>(buffer[pos]))) {
                // Skip if not a digit
                ++pos;
                continue;
            }

            // Read number
            long v = 0;
            while (true) {
                while (pos < buffer.size() &&
                       std::isdigit(static_cast<unsigned char>(buffer[pos]))) {
                    v = v * 10 + (buffer[pos] - '0');
                    ++pos;
                }

                // Exit: Non-digit
                if (pos < buffer.size() &&
                    !std::isdigit(static_cast<unsigned char>(buffer[pos]))) {
                    break;
                }

                // Exit: End of the buffer
                if (pos >= buffer.size()) {
                    // Try to read more
                    if (!fillBuffer()) {
                        // If no more data: End
                        break;
                    }
                    // buffer has grown read more digits
                }
            }

            value = neg ? -v : v;
            return true;
        }
    }

private:
    SOCKET sock;
    std::string buffer;
    size_t pos;

    void skipSpaces() {
        while (pos < buffer.size() &&
               std::isspace(static_cast<unsigned char>(buffer[pos]))) {
            ++pos;
        }
    }

    bool fillBuffer() {
        char temp[4096];
        int n = recv(sock, temp, sizeof(temp), 0);
        if (n <= 0) {
            return false; // Connection closed or error
        }
        buffer.append(temp, n);
        return true;
    }
};


int main() {
    //Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    //Create socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Fill in server address
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP); // Local 127.0.0.1

    // Connect to server
    if (connect(sock, reinterpret_cast<sockaddr*>(&serverAddr),
                sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "connect failed: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server " << SERVER_IP << ":" << SERVER_PORT << "\n";

    // Send group name
    {
        std::string name = MY_GROUP_NAME;
        name.push_back('\n');
        int sent = send(sock, name.c_str(), static_cast<int>(name.size()), 0);
        if (sent <= 0) {
            std::cerr << "Failed to send group name.\n";
            closesocket(sock);
            WSACleanup();
            return 1;
        }
    }

    // Create stream-based integer reader
    IntStreamReader reader(sock);

    // Receive challenges and respond
    while (true) {
        int challengeId = 0;
        int N = 0;

        // Read challenge id and N
        if (!reader.readInt(challengeId)) {
            std::cerr << "Connection closed while reading challenge id.\n";
            break;
        }

        if (!reader.readInt(N)) {
            std::cerr << "Connection closed while reading N.\n";
            break;
        }

        if (N <= 0) {
            std::cerr << "Invalid N = " << N << ", abort.\n";
            break;
        }

        // Allocate space for A and B
        std::vector<int> A(N * N);
        std::vector<int> B(N * N);

        // Read A
        for (int i = 0; i < N * N; ++i) {
            if (!reader.readInt(A[i])) {
                std::cerr << "Connection closed while reading matrix A.\n";
                goto cleanup;
            }
        }

        // Read B
        for (int i = 0; i < N * N; ++i) {
            if (!reader.readInt(B[i])) {
                std::cerr << "Connection closed while reading matrix B.\n";
                goto cleanup;
            }
        }

        // Compute trace of C = A * B mod MODULO
        // trace(C) = sum_i sum_k A[i,k] * B[k,i]
        int trace = 0;

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < N; ++i) {
            int diag = 0;
            const int rowOffsetA = i * N;
            for (int k = 0; k < N; ++k) {
                int a = A[rowOffsetA + k];   // A[i][k]
                int b = B[k * N + i];        // B[k][i]
                int prod = (a * b) % MODULO;
                diag += prod;
                if (diag >= 1'000'000'000) {
                    diag %= MODULO;
                }
            }
            trace = (trace + (diag % MODULO)) % MODULO;
        }

        int answer = trace % MODULO;

        auto end = std::chrono::high_resolution_clock::now();
        auto compute_ms =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Send the answer
        char outBuf[64];
        int len = std::snprintf(outBuf, sizeof(outBuf), "%d\n", answer);
        int sent = send(sock, outBuf, len, 0);
        if (sent <= 0) {
            std::cerr << "Failed to send answer.\n";
            break;
        }

        std::cout << "Challenge " << challengeId
                  << " done, N = " << N
                  << ", answer = " << answer
                  << ", compute time = " << compute_ms << " us\n";
    }

cleanup:
    closesocket(sock);
    WSACleanup();
    std::cout << "Client terminated.\n";
    return 0;
}
