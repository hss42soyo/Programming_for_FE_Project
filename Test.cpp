#include<iostream>
#include <chrono>
using namespace std;
int main()
{
    auto start = chrono::high_resolution_clock::now();
    cout << "Hello my friends, this is a test program." << endl;
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration = end - start;
    cout << "运行时长: " << duration.count() << " ms" << endl;
    return 0; 
}