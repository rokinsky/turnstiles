#ifndef SRC_TURNSTILE_H_
#define SRC_TURNSTILE_H_

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

struct Turnstile {
  std::condition_variable cv;
  bool release;
  uint32_t waits;

  Turnstile() : release(false), waits(0) {}
};

class Mutex {
 private:
  Turnstile* t;

 public:
  Mutex();
  Mutex(const Mutex&) = delete;

  void lock();
  void unlock();
};

#endif /* SRC_TURNSTILE_H_ */
