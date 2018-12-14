#include "turnstile.h"

#include <iostream>
#include <memory>

const uint32_t TC_TABLESIZE = 256; /* Must be power of 2. */

struct Chain {
  std::mutex guard;
  std::queue<std::unique_ptr<Turnstile>> free;

  Chain() = default;
};

std::once_flag init;
std::vector<Chain> turnstile_chains;
std::unique_ptr<Turnstile> special;

inline uintptr_t tc_hash(Mutex *m) {
  return (((uintptr_t)(m) >> 8) & (TC_TABLESIZE - 1));
}

Chain *tc_lookup(Mutex *m) {
  std::call_once(init, []() {
    turnstile_chains = std::vector<Chain>(TC_TABLESIZE);
    special.reset(new Turnstile);
  });
  return &turnstile_chains[tc_hash(m)];
}

Mutex::Mutex() : t(nullptr) {}

thread_local std::once_flag t_init;
thread_local std::unique_ptr<Turnstile> turnstile;

void Mutex::lock() {
  /* For each thread a turnstile is allocated one time and attached to them */
  std::call_once(t_init, []() { turnstile.reset(new Turnstile); });

  auto tc = tc_lookup(this);

  std::unique_lock<std::mutex> lk(tc->guard);

  if (t == nullptr) {
    t = special.get();
  } else {

    if (t == special.get()) {
      t = turnstile.release();
    } else {
      tc->free.push(std::move(turnstile));
    }

    t->waits++;

    t->cv.wait(lk, [&]() { return t->release == true; });
    t->release = false;

    t->waits--;

    if (t->waits > 0) { /* If there are any other waiters, */
      /* it takes a turnstile from the free list */
      turnstile = std::move(tc->free.front());
      tc->free.pop();
    } else { /* If it is the only thread blocked on the lock, */
      /* then it reclaims the turnstile associated with the lock */
      /* and removes it from the hash table. */
      turnstile.reset(t);
      t = special.get();
    }
  }
}

void Mutex::unlock() {
  auto tc = tc_lookup(this);

  std::unique_lock<std::mutex> lk(tc->guard);

  if (t == special.get()) {
    t = nullptr;
  } else {
    t->release.store(true);
    t->cv.notify_one();
  }
}