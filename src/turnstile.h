#ifndef SRC_TURNSTILE_H_
#define SRC_TURNSTILE_H_

#include <thread>
#include <mutex>
#include <unordered_map>
#include <atomic>
#include <queue>

struct Turnstile {
    std::mutex guard;

    Turnstile() {
        guard.lock();
    }
};

class Mutex {
private:
    std::atomic<bool> locked;       /* 1 byte */
    std::atomic<bool> first;        /* 1 byte */
    std::atomic<uint32_t> waits;    /* 4 bytes */

    bool try_lock();
    bool try_first();
    bool has_waits();
public:
    Mutex() : locked(false), first(true), waits(0) {}
    Mutex(const Mutex&) = delete;

    void lock();    // NOLINT
    void unlock();  // NOLINT
};

class Chain {
public:
    std::mutex guard;
    std::unordered_map<Mutex*, Turnstile*> blocked;
    std::queue<Turnstile*> free;

    Chain() = default;
};

#endif  // SRC_TURNSTILE_H_