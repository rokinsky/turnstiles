#include "turnstile.h"

/*
 * Global protection for Mutexes.
 * Try to distribute Mutexes evenly among std::mutex
 */
const uint32_t M_TABLESIZE = 1024;
const uint32_t M_MASK = M_TABLESIZE - 1;

std::array<std::mutex, M_TABLESIZE> M;

inline static std::uintptr_t m_hash(Mutex *m) {
  return reinterpret_cast<std::uintptr_t>(m) & M_MASK;
}

inline static std::mutex *m_lookup(Mutex *m) { return &M[m_hash(m)]; }

/*
 * For each thread
 * a turnstile is allocated one time
 * and attached to them.
 */
thread_local std::unique_ptr<Turnstile> t_turnstile;
thread_local std::once_flag init_t;

/*
 * When a lock references here,
 * it means that the lock is ready to block threads,
 * but hasn't turnstile yet.
 * The first thread global there will allocate it.
 */
std::unique_ptr<Turnstile> ready;
std::once_flag init;

inline void initialization() {
  std::call_once(init_t, []() {
    t_turnstile.reset(new Turnstile);
    std::call_once(init, []() { ready.reset(new Turnstile); });
  });
}

Mutex::Mutex() : m_turnstile(nullptr) {}

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
  initialization();

  auto m = m_lookup(this);

  std::unique_lock<std::mutex> lk(*m);

  if (m_turnstile == nullptr) {
    m_turnstile = ready.get();
  } else {
    if (m_turnstile == ready.get()) {
      m_turnstile = t_turnstile.release();
    } else {
      m_turnstile->free.push(std::move(t_turnstile));
    }

    m_turnstile->cv.wait(lk, [&]() { return m_turnstile->release; });
    m_turnstile->release = false;

    if (!m_turnstile->free.empty()) {
      t_turnstile = std::move(m_turnstile->free.front());
      m_turnstile->free.pop();
    } else {
      t_turnstile.reset(m_turnstile);
      m_turnstile = ready.get();
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
  auto m = m_lookup(this);

  std::lock_guard<std::mutex> lk(*m);

  if (m_turnstile == ready.get()) {
    m_turnstile = nullptr;
  } else if (m_turnstile != nullptr) {
    m_turnstile->release = true;
    m_turnstile->cv.notify_one();
  }
}
