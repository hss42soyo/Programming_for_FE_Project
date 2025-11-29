Compile:
For linux:
Server
g++ -std=c++17 -O2 server_Linux.cpp -lws2_32 -o server_Linux.exe
Client
g++ -std=c++17 -O2 cilent_op_Linux.cpp -lws2_32 -o client_op_Linux.exe

For Windows:
Server
g++ -std=c++17 -O2 server.cpp -lws2_32 -o server.exe
Client
g++ -std=c++17 -O2 client_op.cpp -lws2_32 -o client_op.exe

Execution：
For linux:
Server
./server_Linux.exe
Client
./cilent_op_Linux.exe

For Windows:
Server
./server.exe
Client
./client_op.exe