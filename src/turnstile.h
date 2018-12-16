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
  bool release = false;
  std::queue<std::unique_ptr<Turnstile>> free;
};

class Mutex {
 private:
  Turnstile* m_turnstile;

 public:
  Mutex();
  Mutex(const Mutex&) = delete;

  void lock();
  void unlock();
};

#endif /* SRC_TURNSTILE_H_ */
