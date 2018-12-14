#include "turnstile.h"

const uint32_t TC_TABLESIZE = 256; /* Must be power of 2. */

struct Chain {
  std::mutex guard;
  std::queue<std::unique_ptr<Turnstile>> free;

  Chain() = default;
};

std::once_flag init;
std::vector<Chain> turnstile_chains;

/*
 * When Mutex's pointer t references here,
 * it means that Mutex is ready to block threads,
 * but object hasn't turnstile yet.
 */
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

void Mutex::lock() {
  /* For each thread a turnstile is allocated one time and attached to them */
  thread_local static std::unique_ptr<Turnstile> turnstile(new Turnstile);

  auto tc = tc_lookup(this);

  std::unique_lock<std::mutex> lk(tc->guard);

  if (t == nullptr) { /* If a thread is first inside, */
    /* then makes object ready to block threads. */
    t = special.get();
  } else {
    if (t == special.get()) { /* If it is the first thread to block, */
      /* then it lends its turnstile to the lock. */
      t = turnstile.release();
    } else { /* If the lock already has a turnstile, */
      /* then it gives its turnstile to the chain's turnstile's free list. */
      tc->free.push(std::move(turnstile));
    }

    /* A thread goes to sleep... */
    t->waits++;

    t->cv.wait(lk, [&]() { return t->release; });
    t->release = false;

    t->waits--;
    /* When a thread is woken up: */

    if (t->waits > 0) { /* If there are any other waiters, */
      /* it takes a turnstile from the free list */
      turnstile = std::move(tc->free.front());
      tc->free.pop();
    } else { /* If it is the only thread blocked on the lock, */
      /* then it reclaims the turnstile associated with the lock */
      /* and makes object ready to block threads. */
      turnstile.reset(t);
      t = special.get();
    }
  }
}

void Mutex::unlock() {
  auto tc = tc_lookup(this);

  std::lock_guard<std::mutex> lk(tc->guard);

  if (t == special.get()) { /* If object hasn't a turnstile */
    t = nullptr;
  } else {
    t->release = true;
    t->cv.notify_one();
  }
}
