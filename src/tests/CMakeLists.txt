add_executable(dummy_test dummy_test.cpp)
target_link_libraries(dummy_test turnstile_lib)
target_link_libraries(dummy_test "${CMAKE_THREAD_LIBS_INIT}")
add_test(dummy_test dummy_test)
