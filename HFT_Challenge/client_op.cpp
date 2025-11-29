// client_op_optimized.cpp - Windows HFT Matrix Challenge client

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
#include <thread>
#include <algorithm>

#pragma comment(lib, "Ws2_32.lib")

// configuration
const char* SERVER_IP   = "127.0.0.1";
const int   SERVER_PORT = 12345;
const int   MODULO      = 997;
const std::string MY_GROUP_NAME = "Group1_op";

// Maximum threads
const unsigned int MAX_THREADS = 4;

// Read int
class IntStreamReader {
public:
    explicit IntStreamReader(SOCKET s) : sock(s), pos(0) {}

    // Read an int from the stream, return true on success, false on failure (disconnect/error)
    bool readInt(int& value) {
        while (true) {
            // Skip whitespace
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
                    // buffer has grown; read more digits
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
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    // Create socket
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

    // Preallocate / reuse memory regions
    std::vector<int> A;
    std::vector<int> B;
    std::vector<int> B_T;              // Only used for large matrices + multithreading
    std::vector<int> partial_sums(MAX_THREADS, 0); // Only use the first num_threads elements
    // Continuously receive challenges and respond
    while (true) {
        int challengeId = 0;
        int N = 0;

        // First read challenge id and N
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

        const size_t needed = static_cast<size_t>(N) * static_cast<size_t>(N);
        // Only increase size to avoid repeated allocations
        if (A.size() < needed) {
            A.resize(needed);
            B.resize(needed);
            B_T.resize(needed);
        }

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

        auto start = std::chrono::high_resolution_clock::now();

        int answer = 0;

        // N <= 128: single-threaded + no transpose
        if (N <= 128) {
            int trace = 0;
            for (int i = 0; i < N; ++i) {
                long long diag = 0;
                int rowOffsetA = i * N;
                for (int k = 0; k < N; ++k) {
                    int a = A[rowOffsetA + k];   // A[i][k]
                    int b = B[k * N + i];        // B[k][i]
                    diag += 1LL * a * b;
                }
                diag %= MODULO;
                trace += static_cast<int>(diag);
                if (trace >= 1'000'000'000) trace %= MODULO;
            }
            answer = trace % MODULO;
        } else {
            // N > 128: multi-threaded + transpose B
            // B_T
            for (int k = 0; k < N; ++k) {
                int rowOffsetB = k * N;
                for (int i = 0; i < N; ++i) {
                    // B[k][i] -> B_T[i][k]
                    B_T[i * N + k] = B[rowOffsetB + i];
                }
            }

            unsigned int hw_threads = std::thread::hardware_concurrency();
            if (hw_threads == 0) hw_threads = 2;  // fallback

            unsigned int num_threads = hw_threads;
            if (num_threads > MAX_THREADS) {
                num_threads = MAX_THREADS;
            }
            if (num_threads > static_cast<unsigned int>(N)) {
                num_threads = static_cast<unsigned int>(N);
            }

            if (num_threads <= 1) {
                int trace = 0;
                for (int i = 0; i < N; ++i) {
                    long long diag = 0;
                    int rowOffsetA  = i * N;
                    int rowOffsetBT = i * N;
                    for (int k = 0; k < N; ++k) {
                        int a = A[rowOffsetA + k];
                        int b = B_T[rowOffsetBT + k]; // B_T[i][k] = B[k][i]
                        diag += 1LL * a * b;
                    }
                    diag %= MODULO;
                    trace += static_cast<int>(diag);
                    if (trace >= 1'000'000'000) trace %= MODULO;
                }
                answer = trace % MODULO;
            } else {
                // Multi-threaded computation
                std::vector<std::thread> workers;
                workers.reserve(num_threads);

                int rows_per_thread = (N + num_threads - 1) / num_threads;

                for (unsigned int t = 0; t < num_threads; ++t) {
                    int start_row = t * rows_per_thread;
                    int end_row   = std::min(N, start_row + rows_per_thread);

                    if (start_row >= N) {
                        partial_sums[t] = 0;
                        continue;
                    }

                    workers.emplace_back([&, t, start_row, end_row]() {
                        int local_sum = 0;

                        for (int i = start_row; i < end_row; ++i) {
                            long long diag = 0;
                            int rowOffsetA  = i * N;
                            int rowOffsetBT = i * N;  // row i of B_T

                            for (int k = 0; k < N; ++k) {
                                int a = A[rowOffsetA + k];    // A[i][k]
                                int b = B_T[rowOffsetBT + k]; // B_T[i][k] = B[k][i]
                                diag += 1LL * a * b;
                            }
                            diag %= MODULO;
                            local_sum += static_cast<int>(diag);
                            if (local_sum >= 1'000'000'000) {
                                local_sum %= MODULO;
                            }
                        }

                        partial_sums[t] = local_sum % MODULO;
                    });
                }

                for (auto& th : workers) {
                    if (th.joinable()) th.join();
                }

                int trace = 0;
                for (unsigned int t = 0; t < num_threads; ++t) {
                    trace += partial_sums[t];
                    if (trace >= 1'000'000'000) trace %= MODULO;
                }
                answer = trace % MODULO;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto compute_us =
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
                  << ", compute time = " << compute_us << " us";
        if (N > 128) {
            unsigned int hw_threads = std::thread::hardware_concurrency();
            if (hw_threads == 0) hw_threads = 2;
            unsigned int num_threads = hw_threads;
            if (num_threads > MAX_THREADS) num_threads = MAX_THREADS;
            if (num_threads > static_cast<unsigned int>(N)) {
                num_threads = static_cast<unsigned int>(N);
            }
            std::cout << " (threads used: " << (N > 128 && num_threads > 1 ? num_threads : 1) << ")";
        }
        std::cout << "\n";
    }

cleanup:
    closesocket(sock);
    WSACleanup();
    std::cout << "Client terminated.\n";
    return 0;
}
