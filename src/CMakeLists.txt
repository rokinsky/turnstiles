cmake_minimum_required (VERSION 3.1)
project (MIMUW-CPCPP-lab2 CXX)

find_package ( Threads REQUIRED )

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-g -Wall")

# http://stackoverflow.com/questions/10555706/
macro (add_executable _name)
    # invoke built-in add_executable
    _add_executable(${ARGV})
    if (TARGET ${_name})
        target_link_libraries(${_name} ${CMAKE_THREAD_LIBS_INIT} )
    endif()
endmacro()

add_library(turnstile_lib turnstile.cpp)
add_subdirectory(src/tests)

add_executable(ts turnstile.cpp turnstile.h)

install(TARGETS DESTINATION .)
