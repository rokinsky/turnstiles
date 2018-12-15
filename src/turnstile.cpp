#include "turnstile.h"

const uint32_t TC_TABLESIZE = 256;

struct Chain {
  std::mutex cv_m;
  std::queue<std::unique_ptr<Turnstile>> free;

  Chain() = default;
};

/* First thread allocates global synchronized structure. */
std::once_flag init;
std::vector<Chain> turnstile_chains;

/* For each thread a turnstile is allocated one time and attached to them. */
thread_local std::once_flag init_t;
thread_local std::unique_ptr<Turnstile> turnstile;

/*
 * When Mutex's pointer t references here,
 * it means that Mutex is ready to block threads,
 * but object hasn't turnstile yet.
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

void Mutex::lock() {
  auto tc = tc_lookup(this);

  std::unique_lock<std::mutex> lk(tc->cv_m);

  if (t == nullptr) { /* If a thread is first inside, */
    /* then makes object ready to block threads. */
    t = ready.get();
  } else {
    if (t == ready.get()) { /* If it is the first thread to block, */
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
      t = ready.get();
    }
  }
}

void Mutex::unlock() {
  auto tc = tc_lookup(this);

  std::lock_guard<std::mutex> lk(tc->cv_m);

  if (t == ready.get()) { /* If object hasn't a turnstile */
    t = nullptr;
  } else if (t != nullptr) {
    t->release = true;
    t->cv.notify_one();
  }
}
