#include "turnstile.h"

#include <iostream>
#include <map>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

static_assert(std::is_destructible<Mutex>::value,
              "Mutex should be destriuctible.");
static_assert(!std::is_copy_constructible<Mutex>::value,
              "Mutex should not be copy-constructible.");
static_assert(!std::is_move_constructible<Mutex>::value,
              "Mutex should not be move-constructible.");
static_assert(std::is_default_constructible<Mutex>::value,
              "Mutex should be default constructible.");
static_assert(std::is_same<void, decltype(std::declval<Mutex>().lock())>::value,
              "Mutex should have a \"void lock()\" member function.");
static_assert(
        std::is_same<void, decltype(std::declval<Mutex>().unlock())>::value,
        "Mutex should have a \"void unlock()\" member function.");
static_assert(sizeof(Mutex) <= 8, "Mutex is too large");

thread_local std::string t_name;

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

        int local = shared;
        local += 1;
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        shared = local;
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

        int local = shared3;
        local += 1;
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        shared3 = local;
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

        int local = shared2;
        local += 1;
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        shared2 = local;
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

    const int N = 100;
    int loops = 0;

    std::vector<std::thread> threads;
    std::cout << "size: " << sizeof(Mutex) << std::endl;

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

    std::cout << "result is correct? " << (loop_rep * N * loops == shared + shared2 + shared3) << std::endl;
}