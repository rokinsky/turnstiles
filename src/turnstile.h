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
  std::atomic<bool> release{false};
  std::atomic<uint32_t> waits{0};
  std::mutex cv_m;
  bool CAS(bool expected, bool desired);
};

class Mutex {
 private:
  std::atomic<Turnstile*> m_turnstile;
  bool CAS(void* expected, void* desired);

 public:
  Mutex();
  Mutex(const Mutex&) = delete;

  void lock();
  void unlock();
};

#endif /* SRC_TURNSTILE_H_ */
