/*
 * SPSCQueueRB.hpp
 *
 *  Created on: Mar 23, 2025
 *      Author: Haseeb
 */

 #ifndef SYSTEM_SPSCQUEUERB_HPP_
 #define SYSTEM_SPSCQUEUERB_HPP_
 
 #include <array>
 #include <cstddef>
 #include <atomic>
 
 template<typename T, std::size_t Capacity>
 class SPSCQueueRB {
   static_assert((Capacity & (Capacity - 1)) == 0 , "Capacity must be power of two for this implementation");
 
 public:
   SPSCQueueRB() :
       writeIndex_(0), readIndex_(0) {
 
   }
 
   std::uint8_t push(const T &value) {
     const size_t wr = writeIndex_.load(std::memory_order_relaxed);
     const size_t rd = readIndex_.load(std::memory_order_acquire);
 
     //next index after we insert the item
     const size_t next = (wr + 1) & (Capacity - 1);
 
     //if next == rd, the buffer is full
 
     if (next == rd) {
       return 0; //full
     }
 
     buffer_[wr] = value;
     writeIndex_.store(next, std::memory_order_release);
     return 1;
   }
 
   std::uint8_t pop(T &out) {
     const size_t rd = readIndex_.load(std::memory_order_relaxed);
     const size_t wr = writeIndex_.load(std::memory_order_acquire);
 
     if (rd == wr) {
       return 0; //empty
     }
 
     out = buffer_[rd];
 
     const size_t next = (rd + 1) & (Capacity - 1);
 
     readIndex_.store(next, std::memory_order_release);
   }
 
   /*Multi push implementation*/
   std::size_t multiPush(const T *src, std::size_t count) {
     std::size_t pushed = 0;
     while (pushed < count) {
       if (!push(src[pushed])) {
         break; // buffer full
       }
       ++pushed;
     }
 
     return pushed;
 
   }
 
   /*Multi pop implementation*/
   std::size_t multiPop(T* dst, std::size_t count) {
     std::size_t popped = 0;
 
     while (popped < count) {
       if (!pop(dst[popped])) {
 
       }
 
       ++popped;
     }
     return popped;
 
   }
 
   std::size_t size() const {
     const size_t wr = writeIndex_.load(std::memory_order_acquire);
     const size_t rd = readIndex_.load(std::memory_order_acquire);
     return (wr - rd) & (Capacity - 1);
   }
 
   std::size_t vacant() const {
     return (Capacity - 1) - size();
   }
 
   constexpr std::size_t capacity() const {
     return Capacity;
   }
 
   std::uint8_t empty() const {
     const size_t wr = writeIndex_.load(std::memory_order_acquire);
     const size_t rd = readIndex_.load(std::memory_order_acquire);
     return (wr == rd);
   }
 
   std::uint8_t full() const {
     const size_t wr = writeIndex_.load(std::memory_order_acquire);
     const size_t rd = readIndex_.load(std::memory_order_acquire);
     const size_t next = (wr + 1) & (Capacity - 1);
     return (next == rd);
   }
 
   void clear()
   {
        writeIndex_.store(0, std::memory_order_relaxed);
             readIndex_.store(0, std::memory_order_relaxed);
   }
 
 
       void flush()
       {
           clear();
       }
 
 
      std::size_t search(const T& value) const
      {
        size_t rd = readIndex_.load(std::memory_order_acquire);
        const size_t wr = writeIndex_.load(std::memory_order_acquire);
 
        std::size_t position = 0;
 
           while (rd != wr) {
               ++position;
               if (buffer_[rd] == value) {
                   return position;  // found
               }
               rd = (rd + 1) & (Capacity - 1);
           }
           return 0; // not found
      }
 
 
 private:
   std::array<T, Capacity> buffer_;
   std::atomic<size_t> writeIndex_;
   std::atomic<size_t> readIndex_;
 
   /*Non copyable*/
   SPSCQueueRB(const SPSCQueueRB&) = delete;
   SPSCQueueRB& operator=(const SPSCQueueRB&) = delete;
 
   /*Non movable*/
   SPSCQueueRB(const SPSCQueueRB&&) = delete;
   SPSCQueueRB& operator=(const SPSCQueueRB&&) = delete;
 
 };
 
 #endif /* SYSTEM_SPSCQUEUERB_HPP_ */
 