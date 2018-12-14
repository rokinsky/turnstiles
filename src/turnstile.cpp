#include "turnstile.h"

#include <iostream>

const uint32_t TC_TABLESIZE = 256; /* Must be power of 2. */

std::once_flag init;

struct Turnstile {
  std::mutex guard;
  std::condition_variable cv;
  std::mutex cv_m;
  std::atomic<bool> release{false};

  Turnstile() { guard.lock(); }
};

struct Chain {
  std::mutex guard;
  std::unordered_map<Mutex *, std::shared_ptr<Turnstile>> blocked;
  std::queue<std::shared_ptr<Turnstile>> free;

  Chain() = default;
};

std::vector<Chain> turnstile_chains;

inline uintptr_t tc_hash(Mutex *m) {
  return (((uintptr_t)(m) >> 8) & (TC_TABLESIZE - 1));
}

Chain *tc_lookup(Mutex *m) {
  std::call_once(init,
                 []() { turnstile_chains = std::vector<Chain>(TC_TABLESIZE); });
  return &turnstile_chains[tc_hash(m)];
}

Mutex::Mutex() : locked(false), first(true), waits(0) {}

bool Mutex::try_lock() {
  bool expected = false;
  return locked.compare_exchange_weak(expected, true);
}

bool Mutex::try_first() {
  bool expected = true;
  return first.compare_exchange_weak(expected, false);
}

bool Mutex::has_waits() { return waits > 0; }

thread_local std::shared_ptr<Turnstile> turnstile;
thread_local std::once_flag t_init;
thread_local bool has_turnstile;

void Mutex::lock() {
  /* For each thread a turnstile is allocated one time and attached to them */
  std::call_once(t_init, [&]() { turnstile = std::make_shared<Turnstile>(); has_turnstile = true;});

  auto tc = tc_lookup(this);

//  std::unique_lock<std::mutex> lk(tc->guard);
  tc->guard.lock();

  if (!try_lock()) { /* If some thread was there */
    std::shared_ptr<Turnstile> t;
    if (try_first()) { /* if it is the first thread to block, */
      /* it lends its turnstile to the lock. */
      tc->blocked[this] = t = turnstile;
    } else { /* If the lock already has a turnstile, */
      /* then it gives its turnstile to the lock's turnstile's free list. */
      t = tc->blocked[this];
      tc->free.push(turnstile);
    }

    has_turnstile = false;

    waits++;

    //t->cv.wait(lk, [&]() { return t->release == true; });
    //t->release = false;
    tc->guard.unlock();
    t->guard.lock();

    waits--;
  }

  tc->guard.unlock();
}

void Mutex::unlock() {
  auto tc = tc_lookup(this);

  //std::lock_guard<std::mutex> lk(tc->guard);

  tc->guard.lock();

  if (!has_waits()) {
    if (!has_turnstile) {
      has_turnstile = true;

      turnstile = tc->blocked[this];
      tc->blocked.erase(this);
      first.store(true);
    }

    locked.store(false);

    tc->guard.unlock();
  } else {
    if (!has_turnstile) {
      has_turnstile = true;

      turnstile = tc->free.front();
      tc->free.pop();
    }

    auto t = tc->blocked[this];
    //t->release.store(true);
    //t->cv.notify_one();
    t->guard.unlock();
  }
}
