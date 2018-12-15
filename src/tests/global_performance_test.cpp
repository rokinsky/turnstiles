#include "turnstile.h"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
#include <shared_mutex>

template <typename M>
class CriticalSection {
public:

    explicit CriticalSection(long cost) : cost(cost) {}

    CriticalSection(const CriticalSection& other) noexcept : counter(other.counter), cost(other.cost) {}

    void add(unsigned x) {
        std::unique_lock<M> lock{mutex};
        counter += x;
        std::this_thread::sleep_for(std::chrono::milliseconds(cost));
    }

    unsigned getCounter() {
        return counter;
    }

private:

    unsigned counter{0};
    long cost;
    M mutex{};
};

std::shared_timed_mutex blockade;

template <typename M>
void addToSection(CriticalSection<M>& criticalSection, size_t repeatsPerThread) {
    std::shared_lock<std::shared_timed_mutex> lock {blockade};
    for(size_t i = 0; i < repeatsPerThread; ++i) {
        criticalSection.add(1);
    }
}

template <typename M>
void test(size_t criticalSections, size_t threadsPerSection, size_t repeatsPerThread, long cost) {
    std::cout << "Beginning test with " << criticalSections << " sections, " << threadsPerSection * criticalSections
              << " threads and total of " << threadsPerSection * criticalSections * repeatsPerThread
              << " portions with a cost of " << cost << std::endl;

    std::vector<CriticalSection<M>> sections;
    std::vector<std::thread> threads;

    for(size_t i = 0; i < criticalSections; ++i) {
        sections.emplace_back(cost);
    }

    std::unique_lock<std::shared_timed_mutex> lock {blockade};

    for(auto& section : sections) {
        for (size_t j = 0; j < threadsPerSection; ++j) {
            threads.emplace_back([&section, repeatsPerThread] { addToSection<M>(section, repeatsPerThread); });
        }
    }

    std::cout << "Blockade down." << std::endl;
    lock.unlock();

    for(auto& thread : threads) {
        thread.join();
    }

    for(auto& section : sections) {
        assert(section.getCounter() == threadsPerSection * repeatsPerThread);
        section.getCounter();
    }
}

int main() {
    auto startStd = std::chrono::high_resolution_clock::now();
    test<std::mutex>(1000, 4, 1000, 2);
    auto stopStd = std::chrono::high_resolution_clock::now();
    auto startMutex = std::chrono::high_resolution_clock::now();
    test<Mutex>(1000, 4, 1000, 2);
    auto stopMutex = std::chrono::high_resolution_clock::now();

    std::cout << "std::mutex time: " << (stopStd - startStd).count() * 0.000000001 << " s" << std::endl;
    std::cout << "Mutex time:      " << (stopMutex - startMutex).count() * 0.000000001 << " s" << std::endl;
}