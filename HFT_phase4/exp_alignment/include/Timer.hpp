#pragma once
#include <chrono>

class Timer {
public:
   void start() { start_ = std::chrono::high_resolution_clock::now(); }
   long long stop_ns() const {
      auto end = std::chrono::high_resolution_clock::now();
      return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_).count();
   }
private:
   std::chrono::high_resolution_clock::time_point start_;
};
