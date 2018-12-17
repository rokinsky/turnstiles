#include "turnstile.h"

/*
 * When a lock references here,
 * it means that the lock is ready to block threads,
 * but hasn't turnstile yet.
 * The first thread global there will allocate it.
 */
std::unique_ptr<Turnstile> ready_turnstile;
std::once_flag init;

Mutex::Mutex() : m_turnstile(nullptr) {}

bool Mutex::CAS(void* expected, void* desired) {
  auto exp = static_cast<Turnstile*>(expected);
  auto dsr = static_cast<Turnstile*>(desired);
  return m_turnstile.compare_exchange_strong(exp, dsr,
                                             std::memory_order_acquire);
}

Turnstile* Mutex::turnstile() {
  return m_turnstile.load();
}

bool Mutex::has_waits() {
  return turnstile()->waits > 0;
}

bool Turnstile::CAS(bool expected, bool desired) {
  return release.compare_exchange_strong(expected, desired,
                                         std::memory_order_acquire);
}

/*
 * If a thread is the first there,
 * then it makes Mutex ready to block threads
 * and goes to the critical section...
 *
 * If it is the first thread to block,
 * then it lends its turnstile to Mutex.
 * Else if Mutex already has a turnstile,
 * then it gives its turnstile to blocked turnstile's free list.
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
  std::call_once(init, []() { ready_turnstile.reset(new Turnstile); });

  if (!CAS(nullptr, ready_turnstile.get())) {
    CAS(ready_turnstile.get(), new Turnstile); /* mem leak 100%, but don't care */
    turnstile()->waits++;

    std::unique_lock<std::mutex> lk(turnstile()->cv_m);
    turnstile()->cv.wait(lk, [&]() { return turnstile()->CAS(true, false); });
    turnstile()->waits--;

    if (!has_waits()) { /* here problems */
      Turnstile* tmp = m_turnstile;
      m_turnstile.store(ready_turnstile.get(), std::memory_order_release);
      lk.unlock();
      delete tmp;
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
  if (!CAS(ready_turnstile.get(), nullptr)) {
    std::lock_guard<std::mutex> lk(turnstile()->cv_m);
    turnstile()->release.store(true, std::memory_order_release);
    turnstile()->cv.notify_one();
  }
}
