#pragma once
#include <cstddef>
#include <vector>

// Simple single-threaded fixed-size object pool.
// For benchmarking only.
class FixedBlockPool {
public:
   FixedBlockPool(std::size_t block_size, std::size_t blocks)
      : block_size_(block_size) {
      buf_.resize(block_size_ * blocks);
      free_.reserve(blocks);
      for (std::size_t i = 0; i < blocks; ++i)
         free_.push_back(buf_.data() + i * block_size_);
   }

   void* alloc() {
      if (free_.empty()) return ::operator new(block_size_);
      void* p = free_.back(); free_.pop_back(); return p;
   }
   void dealloc(void* p) {
      // if p inside pool, recycle; else free to global
      auto* base = buf_.data();
      auto* end = base + buf_.size();
      if (p >= base && p < end) free_.push_back((char*)p);
      else ::operator delete(p);
   }

private:
   std::size_t block_size_;
   std::vector<char> buf_;
   std::vector<void*> free_;
};
