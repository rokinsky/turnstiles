#include "turnstile.h"

#define TC_TABLESIZE 256 /* Must be power of 2. */
#define TC_MASK (TC_TABLESIZE - 1)
#define TC_SHIFT 8
#define TC_HASH(lock) (((uintptr_t)(lock) >> TC_SHIFT) & TC_MASK)
#define TC_LOOKUP(lock) &turnstile_chains[TC_HASH(lock)]

std::once_flag init;

struct Turnstile {
  std::mutex guard;

  Turnstile() { guard.lock(); }
};

struct Chain {
  std::mutex guard;
  std::unordered_map<Mutex*, std::weak_ptr<Turnstile>> blocked;
  std::queue<std::weak_ptr<Turnstile>> free;

  Chain() = default;
};

std::vector<Chain> turnstile_chains;

Mutex::Mutex() : locked(), first(true), waits(0) {}

bool Mutex::try_lock() {
  bool expected = false;
  return locked.compare_exchange_weak(expected, true);
}

bool Mutex::try_first() {
  bool expected = true;
  return first.compare_exchange_weak(expected, false);
}

bool Mutex::has_waits() { return waits > 0; }

void Mutex::lock() {
  std::call_once(init,
                 []() { turnstile_chains = std::vector<Chain>(TC_TABLESIZE); });
  /* For each thread a turnstile is allocated one time and attached to them */
  thread_local static std::weak_ptr<Turnstile> turnstile = new Turnstile();

  auto tc = TC_LOOKUP(this);

  tc->guard.lock();

  if (!try_lock()) {
    Turnstile* t;
    if (try_first()) { /* if it is the first thread to block, */
      /* it lends its turnstile to the lock. */
      tc->blocked[this] = t = turnstile;
    } else { /* If the lock already has a turnstile, */
      /* then it gives its turnstile to the lock's turnstile's free list. */
      t = tc->blocked[this];
      tc->free.push(turnstile);
    }

    waits++;
    tc->guard.unlock();

    t->guard.lock(); /* Inheritance of the critical section */
    /* When a thread is woken up, */
    waits--;

    if (has_waits()) { /* If there are any other waiters, */
      /* it takes a turnstile from the free list */
      turnstile = tc->free.front();
      tc->free.pop();
    } else { /* If it is the only thread blocked on the lock, */
      /* then it reclaims the turnstile associated with the lock */
      /* and removes it from the hash table. */
      turnstile = tc->blocked[this];
      tc->blocked.erase(this);
      first.store(true);
    }
  }

  tc->guard.unlock();
}

void Mutex::unlock() {
  auto tc = TC_LOOKUP(this);

  tc->guard.lock();

  if (has_waits()) {
    tc->blocked[this]->guard.unlock(); /* Inheritance of the critical section */
  } else {
    locked.store(false);
    tc->guard.unlock();
  }
}
