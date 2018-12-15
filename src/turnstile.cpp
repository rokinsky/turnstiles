#include "turnstile.h"

const uint32_t TC_TABLESIZE = 256;

struct Chain {
  std::mutex cv_m;
  std::queue<std::unique_ptr<Turnstile>> free;

  Chain() = default;
};

/* The first thread allocates global synchronized structure. */
std::once_flag init;
std::vector<Chain> turnstile_chains;

/* For each thread a turnstile is allocated one time and attached to them. */
thread_local std::once_flag init_t;
thread_local std::unique_ptr<Turnstile> turnstile;

/*
 * When a lock references here,
 * it means that the lock is ready to block threads,
 * but hasn't turnstile yet.
 */
std::unique_ptr<Turnstile> ready;

inline void initialization() {
  std::call_once(init_t, []() {
    turnstile.reset(new Turnstile);
    std::call_once(init, []() {
      turnstile_chains = std::vector<Chain>(TC_TABLESIZE);
      ready.reset(new Turnstile);
    });
  });
}

inline std::uintptr_t tc_hash(Mutex *m) {
  return reinterpret_cast<std::uintptr_t>(m) % TC_TABLESIZE;
}

inline Chain *tc_lookup(Mutex *m) {
  initialization();
  return &turnstile_chains[tc_hash(m)];
}

Mutex::Mutex() : t(nullptr) {}

/*
 * If a thread is the first there,
 * then it makes Mutex ready to block threads
 * and goes to the critical section...
 *
 * If it is the first thread to block,
 * then it lends its turnstile to Mutex.
 * Else if Mutex already has a turnstile,
 * then it gives its turnstile to free list.
 * A thread goes to sleep...
 *
 * When a thread is woken up...
 * If there are any other waiters,
 * it takes a turnstile from the free list.
 * Else if it is the only thread blocked on Mutex,
 * then it reclaims the turnstile associated with Mutex
 * and makes Mutex ready to block threads.
 * A thread goes to the critical section...
 */
void Mutex::lock() {
  auto tc = tc_lookup(this);

  std::unique_lock<std::mutex> lk(tc->cv_m);

  if (t == nullptr) {
    t = ready.get();
  } else {
    if (t == ready.get()) {
      t = turnstile.release();
    } else {
      tc->free.push(std::move(turnstile));
    }

    t->waits++;

    t->cv.wait(lk, [&]() { return t->release; });
    t->release = false;

    t->waits--;

    if (t->waits > 0) {
      turnstile = std::move(tc->free.front());
      tc->free.pop();
    } else {
      turnstile.reset(t);
      t = ready.get();
    }
  }
}

/*
 * If Mutex hasn't a turnstile,
 * but was ready to block threads,
 * then it makes Mutex free.
 * Else if some thread sleeps,
 * then it will wake its up.
 */
void Mutex::unlock() {
  auto tc = tc_lookup(this);

  std::lock_guard<std::mutex> lk(tc->cv_m);

  if (t == ready.get()) {
    t = nullptr;
  } else if (t != nullptr) {
    t->release = true;
    t->cv.notify_one();
  }
}
