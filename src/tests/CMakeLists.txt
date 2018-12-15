add_executable(trivial_test trivial_test.cpp)
target_link_libraries(trivial_test turnstile_lib)
target_link_libraries(trivial_test "${CMAKE_THREAD_LIBS_INIT}")
add_test(trivial_test trivial_test)

add_executable(multiple_test multiple_test.cpp)
target_link_libraries(multiple_test turnstile_lib)
target_link_libraries(multiple_test "${CMAKE_THREAD_LIBS_INIT}")
add_test(multiple_test multiple_test)

add_executable(producer_consumer_test producer_consumer_test.cpp)
target_link_libraries(producer_consumer_test turnstile_lib)
target_link_libraries(producer_consumer_test "${CMAKE_THREAD_LIBS_INIT}")
add_test(producer_consumer_test producer_consumer_test)