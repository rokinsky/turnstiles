#ifndef SRC_TURNSTILE_H_
#define SRC_TURNSTILE_H_

#include <thread>
#include <mutex>
#include <unordered_map>
#include <atomic>
#include <queue>

class Mutex {
private:
    std::atomic<bool> locked;       // 1 byte
    std::atomic<bool> first;        // 1 byte
    std::atomic<uint32_t> waits;    // 4 bytes

    bool try_lock();
    bool try_first();
    bool has_waits();
public:
    Mutex();
    Mutex(const Mutex&) = delete;

    void lock();    // NOLINT
    void unlock();  // NOLINT
};

#endif  // SRC_TURNSTILE_H_