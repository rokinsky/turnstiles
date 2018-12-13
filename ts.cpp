#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include "log.h"
#include <atomic>
#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <cassert>
#include <queue>

#define	TC_TABLESIZE	256			/* Must be power of 2. */
#define	TC_MASK		(TC_TABLESIZE - 1)
#define	TC_SHIFT	8
#define	TC_HASH(lock)	(((uintptr_t)(lock) >> TC_SHIFT) & TC_MASK)
#define	TC_LOOKUP(lock)	&turnstile_chains[TC_HASH(lock)]

thread_local std::string t_name;

struct Turnstile {
    std::mutex guard;

    Turnstile() {
        guard.lock();
    }
};

class Mutex {
private:
    std::atomic<bool> locked;
    std::atomic<bool> first;
    std::atomic<uint32_t> waits;

    bool try_lock();
    bool try_first();
    bool has_waits();
public:
    Mutex() : locked(false), first(true), waits(0) {}

    void lock();

    void unlock();
};

class Chain {
public:
    std::mutex guard;
    std::unordered_map<Mutex*, Turnstile*> blocked;
    std::queue<Turnstile*> free;

    Chain() = default;
};

Chain turnstile_chains[TC_TABLESIZE]; /* TODO Clang-Tidy: Initialization of 'turnstile_chains' with static storage duration may throw an exception that cannot be caught */

bool Mutex::try_lock() {
    bool expected = false;
    return locked.compare_exchange_weak(expected, true);
}

bool Mutex::try_first() {
    bool expected = true;
    return first.compare_exchange_weak(expected, false);
}

bool Mutex::has_waits() {
    return waits > 0;
}

void Mutex::lock() {
    auto tc = TC_LOOKUP(this);

    thread_local static auto *turnstile = new Turnstile(); /* For each thread a turnstile is allocated one time and attached to them */

    tc->guard.lock();

    if (!try_lock()) {
        Turnstile* t;
        if (try_first()) {  /* if it is the first thread to block, */
            tc->blocked[this] = t = turnstile; /* it lends its turnstile to the lock. */
        } else { /* If the lock already has a turnstile, */
            t = tc->blocked[this];
            tc->free.push(turnstile); /* then it gives its turnstile to the lock's turnstile's free list. */
        }

        turnstile = nullptr;

        waits++;
        tc->guard.unlock();

        t->guard.lock(); /* Inheritance of the critical section */
        /* When a thread is woken up, */
        waits--;

        if (has_waits()) { /* If there are any other waiters, */
            turnstile = tc->free.front(); /* it takes a turnstile from the free list */
            tc->free.pop();
        } else { /* If it is the only thread blocked on the lock, */
            turnstile = tc->blocked[this]; /* then it reclaims the turnstile associated with the lock */
            tc->blocked.erase(this); /* and removes it from the hash table. */
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



/* -------------------------------------------------------------------------------------------------------------- DUPA TEST -------------------------------------------------------------------------------------------------------------- */

int shared{0};
int shared2{0};
int shared3{0};

//std::mutex mut41, mut42, mut43, mut44, mut45, mut46, mut47, mut48, mut49, mut50, mut51, mut52, mut53, mut54, mut55, mut56, mut57, mut58, mut59, mut60, mut61, mut62, mut63, mut64, mut65, mut66, mut67, mut68, mut69, mut70;
//std::mutex mut1, mut2, mut3, mut4, mut5, mut6, mut7, mut8, mut9, mut10, mut11, mut12,mut13,mut14,mut15,mut16,mut17,mut18,mut19,mut20,mut21,mut22,mut23, mut24, mut25, mut26, mut27, mut28, mut29, mut30, mut31, mut32, mut33, mut34, mut35, mut36, mut37, mut38, mut39, mut40;
Mutex mut1, mut2, mut3, mut4, mut5, mut6, mut7, mut8, mut9, mut10, mut11, mut12,mut13,mut14,mut15,mut16,mut17,mut18,mut19,mut20,mut21,mut22,mut23, mut24, mut25, mut26, mut27, mut28, mut29, mut30, mut31, mut32, mut33, mut34, mut35, mut36, mut37, mut38, mut39, mut40;
Mutex mut41, mut42, mut43, mut44, mut45, mut46, mut47, mut48, mut49, mut50, mut51, mut52, mut53, mut54, mut55, mut56, mut57, mut58, mut59, mut60, mut61, mut62, mut63, mut64, mut65, mut66, mut67, mut68, mut69, mut70;

void f(const std::string& name, int loop_rep) {
    t_name = name;
    for (int i = 0; i < loop_rep; i++) {
        mut1.lock();
        mut2.lock();
/*        mut3.lock();
        mut4.lock();
        mut5.lock();
        mut6.lock();
        mut7.lock();
        mut8.lock();
        mut9.lock();
        mut10.lock();
        mut11.lock();
        mut12.lock();
        mut13.lock();
        mut14.lock();
        mut15.lock();
        mut16.lock();
        mut17.lock();
        mut18.lock();
        mut19.lock();
        mut20.lock();
        mut21.lock();
        mut22.lock();
        mut23.lock();
        mut24.lock();
        mut25.lock();
        mut26.lock();
        mut27.lock();
        mut28.lock();
        mut29.lock();
        mut30.lock();
        mut31.lock();
        mut32.lock();
        mut33.lock();
        mut34.lock();
        mut35.lock();
        mut36.lock();
        mut37.lock();
        mut38.lock();
        mut39.lock();
        mut40.lock();*/

        log(i, "f ", name, " critical section start");
        int local = shared;
        local += 1;
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        shared = local;
        log(i, "f ", name, " critical section finish");
/*        mut40.unlock();
        mut39.unlock();
        mut38.unlock();
        mut37.unlock();
        mut36.unlock();
        mut35.unlock();
        mut34.unlock();
        mut33.unlock();
        mut32.unlock();
        mut31.unlock();
        mut30.unlock();
        mut29.unlock();
        mut28.unlock();
        mut27.unlock();
        mut26.unlock();
        mut25.unlock();
        mut24.unlock();
        mut23.unlock();
        mut22.unlock();
        mut21.unlock();
        mut20.unlock();
        mut19.unlock();
        mut18.unlock();
        mut17.unlock();
        mut16.unlock();
        mut15.unlock();
        mut14.unlock();
        mut13.unlock();
        mut12.unlock();
        mut11.unlock();
        mut10.unlock();
        mut9.unlock();
        mut8.unlock();
        mut7.unlock();
        mut6.unlock();
        mut5.unlock();
        mut4.unlock();
        mut3.unlock();*/
        mut2.unlock();
        mut1.unlock();
    }
}


void z(const std::string& name, int loop_rep) {
    t_name = name;

    for (int i = 0; i < loop_rep; i++) {
        mut1.lock();
        mut2.lock();
        mut3.lock();
        mut4.lock();
        mut5.lock();
        mut6.lock();
        mut7.lock();
        mut8.lock();
        mut9.lock();
        mut10.lock();
        mut11.lock();
        mut12.lock();
        mut13.lock();
        mut14.lock();
        mut15.lock();
        mut16.lock();
        mut17.lock();
        mut18.lock();
        mut19.lock();
        mut20.lock();
        mut21.lock();
        mut22.lock();
        mut23.lock();
        mut24.lock();
        mut25.lock();
        mut26.lock();
        mut27.lock();
        mut28.lock();
        mut29.lock();
        mut30.lock();
        mut31.lock();
        mut32.lock();
        mut33.lock();
        mut34.lock();
        mut35.lock();
        mut36.lock();
        mut37.lock();
        mut38.lock();
        mut39.lock();
        mut40.lock();

        log(i, "f ", name, " critical section start");
        int local = shared3;
        local += 1;
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        shared3 = local;
        log(i, "f ", name, " critical section finish");
        mut40.unlock();
        mut39.unlock();
        mut38.unlock();
        mut37.unlock();
        mut36.unlock();
        mut35.unlock();
        mut34.unlock();
        mut33.unlock();
        mut32.unlock();
        mut31.unlock();
        mut30.unlock();
        mut29.unlock();
        mut28.unlock();
        mut27.unlock();
        mut26.unlock();
        mut25.unlock();
        mut24.unlock();
        mut23.unlock();
        mut22.unlock();
        mut21.unlock();
        mut20.unlock();
        mut19.unlock();
        mut18.unlock();
        mut17.unlock();
        mut16.unlock();
        mut15.unlock();
        mut14.unlock();
        mut13.unlock();
        mut12.unlock();
        mut11.unlock();
        mut10.unlock();
        mut9.unlock();
        mut8.unlock();
        mut7.unlock();
        mut6.unlock();
        mut5.unlock();
        mut4.unlock();
        mut3.unlock();
        mut2.unlock();
        mut1.unlock();
    }
}


void x(const std::string& name, int loop_rep) {
    t_name = name;

    for (int i = 0; i < loop_rep; i++) {
        mut41.lock();
        mut42.lock();
        mut43.lock();
        mut44.lock();
        mut45.lock();
        mut46.lock();
        mut47.lock();
        mut48.lock();
        mut49.lock();
        mut50.lock();
        mut51.lock();
        mut52.lock();
        mut53.lock();
        mut54.lock();
        mut55.lock();
        mut56.lock();
        mut57.lock();
        mut58.lock();
        mut59.lock();
        mut60.lock();
        mut61.lock();
        mut62.lock();
        mut63.lock();
        mut64.lock();
        mut65.lock();
        mut66.lock();
        mut67.lock();
        mut68.lock();
        mut69.lock();
        mut70.lock();

        log(i, "f ", name, " critical section start");
        int local = shared2;
        local += 1;
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        shared2 = local;
        log(i, "f ", name, " critical section finish");
        mut70.unlock();
        mut69.unlock();
        mut68.unlock();
        mut67.unlock();
        mut66.unlock();
        mut65.unlock();
        mut64.unlock();
        mut63.unlock();
        mut62.unlock();
        mut61.unlock();
        mut60.unlock();
        mut59.unlock();
        mut58.unlock();
        mut57.unlock();
        mut56.unlock();
        mut55.unlock();
        mut54.unlock();
        mut53.unlock();
        mut52.unlock();
        mut51.unlock();
        mut50.unlock();
        mut49.unlock();
        mut48.unlock();
        mut47.unlock();
        mut46.unlock();
        mut45.unlock();
        mut44.unlock();
        mut43.unlock();
        mut42.unlock();
        mut41.unlock();
    }
}

int main() {
    int loop_rep{10};

    const int N = 10;
    int loops = 0;

    std::vector<std::thread> threads;
    log("size: ", sizeof(Mutex));

    for (int i = 0; i < N; i++) {
        threads.emplace_back( std::thread{[i, loop_rep]{ f("t" + std::to_string(i), loop_rep); }});
    }
    loops++;

    for (int i = 0; i < N; i++) {
        threads.emplace_back( std::thread{[i, loop_rep]{ z("z" + std::to_string(i), loop_rep); }});
    }
    loops++;

    for (int i = 0; i < N; i++) {
        threads.emplace_back( std::thread{[i, loop_rep]{ x("x" + std::to_string(i), loop_rep); }});
    }
    loops++;

    for (auto &t : threads) {
        t.join();
    }

    log("result is correct? ", (loop_rep * N * loops == shared + shared2 + shared3), "");
}