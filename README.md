# Turnstiles

## Motivation
pthread_mutex_t and std::mutex take up 40 bytes of memory. You could imagine a situation, where there are lots of mutexes and, hence, every byte counts.


"Turnstiles" are an example of how one can lower that memory footprint. This idea is used internally in NetBSD and Solaris among others. It hinges on the fact that a single thread cannot sleep on more than one mutex at a time. This fact allows all mutexes to be just placeholders. These placeholders are allocated a state (a "turnstile" in this terminology) on demand - when there is contention and one of the threads has to be suspended. The turnstile is held by the mutex until no more threads need to wait on it. This way, the total number of turnstiles used simultaneously is going to always be lower or equal to the number of threads. This allows for an implementation involving a global pool of turnstiles. A mutex grabs a turnstile from the pool when a thread needs to be suspended and releases the turnstile when no more threads need to sleep on this mutex.

You can find more detailed information about turnstiles online or you can figure it out yourself.
## The task
Your task is to implement a "Mutex" class, which will take up no more then 8 bytes. It should implement the interface defined in "turnstile.h". The total memory consumption beyond the "Mutex" objects should not depend on the number of "Mutex" objects. It may depend on the number of threads, though.

You should not directly use futexes nor inline assembly. Please use std::mutex, std::condition_variable and std::atomic*.
## Technical requirements
Solutions should be delivered in the form described below and should satisfy these requirements. Otherwise they will not be accepted.
* you should clone the git repository https://www.mimuw.edu.pl/~dopiera/turnstiles/
* you should deliver the solution as the output of "git diff" against the tip of the master branch in that repository
"cmake . && make" should compile the solution; a basic test is included in the repository
* by default, cmake will configure compilation for debugging (no optimizations and extra assertions); in order to configure for performance, please run "cmake -DCMAKE_BUILD_TYPE=Release ."
* if you add any files to the solution, please make sure "CMakeLists.txt" is updated too, so that "cmake . && make" still builds the solution
* you should not change the compilation flags in CMakeLists.txt nor the general structure of this file; the "tests" directory * will be replaced for the purpose of testing your solution
* the tests will include "turnstile.h" and link "trunstile_lib" - exactly like "trivial_test" does
* you can and should modify "turnstile.h" - there are static assertions on requirements on the Mutex class in "trivial_test.cpp"
* the solution should compile with g++ 7.3.0 and clang 6.0.0 on Ubuntu 18.04.1 LTS without any extra libraries (except pthreads)
* compilation should not yield any warnings
* the code should be formatted using the "scripts/format.sh" script
* "scripts/tidy.sh" and "scripts/lint.sh" should not generate any warnings (you'll need "clang-tidy" for them to work)
* the test from the aforementioned repository should continue to pass without any modifications
* please use English for variable names, comments, functions, etc.
## Environment
The solution will be tested on Ubuntu. This section outlines how you can set up the envrionment for yourself.

In any Linux distribution please install docker. If you don't use Linux, you can install any distro in a VM.

Create a directory called "turnstile_tester". Inside it, create a "Dockerfile" file, with the following content:
```
FROM ubuntu:18.04

RUN apt-get update && apt-get install -y cmake clang g++ make clang \
    clang-tidy clang-format vim git
```
In "turnstile_tester"''s parent directory run:
```
$ docker build -t turnstile_tester turnstile_tester
```
In order to switch to the environment , run:
```
$ docker run -it -w /root -v "$(pwd):/root" turnstile_tester /bin/bash 
```
## Evaluation
Correctness is going to be the most important factor. Solutions which limit the concurrency beyond what is necessary will be considered wrong. Performance and code quality will also be taken into account.

## FAQ
* This is where the most interesting questions and their answers will appear. Please send them to dopiera@mimuw.edu.pl.
Can I assume that there will not be more than 232 Mutexes?  
NO, let's assume that there can be as many as one can fit on a machine. On large machines 232 might be too few.

* Does the sentence "Solutions which limit the concurrency beyond what is necessary will be considered wrong." mean that there cannot be any global synchronization?  
NO. You can have global, synchronized structures, but please be warned that global synchronization may be costly.
 The intention is such that this code doesn't block:
```
Mutex m1, m2;
m1.lock();
m2.lock();
m2.unlock();
m1.unlock();
```

* Does the sentence "Please use std::mutex, std::condition_variable and std::atomic*" mean that one should not use "thread_local" 
NO. You are allowed to use "thread_local". You are allowed to use any C++ features, but for synchronization, please limit yourself to the aforementioned.

* Does Mutex have to be fair?  
NOT in a strict sense. Please don't make it too unfair, though, .e.g don't favor threads with lower thread ids.

* Shall we ensure that unused "turnstile" objects are removed?  
You should keep the invariant that there are not significantly more turnstiles than threads. If there are going to be fewer of them, it's even better, but please remember that performance will also be graded

* How should one deliver the solution?  
The aforementioned result of "git diff" against the tip of the master branch should be delivered via moodle in a file called "turnstile.diff".

* What pointer size should we assume?  
64 bits.

* Can we assume that std::atomic<> has the same size as the type which it wraps?  
In general no, however for the purpose of thise exercise you can assume it's true for bool int8_t uint8_t int16_t uint16_t int32_t uint32_t int64_t uint64_t.

* How shall we handle improper use of Mutex?  
Any way you wish - it won't be tested.

* How many turnstiles can coexist in a program? Assuming that N is the total number of threads, is it O(N), N+C or something else?  
I'd prefer not to be precise with it. N+C seems to aggressive - I'wouldn't mind e.g. 2N. O(N) seems to relaxed, though, because it could mean 1000000N or N + 1000000000. Please use common sense. In real life both asymptotic complexity and the constants count.

* Why does the aforementioned list of type for which std::atomic size matches the size of underlying type doesn't include pointers?  
It's an oversight. You can assume that std::atomic<T*> has the same size as T*.

Author: Marek Dopiera
