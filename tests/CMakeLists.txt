SET(CMAKE_CXX_FLAGS "-std=c++0x")

add_executable(test_taskqueue taskqueue.cpp)
set(TEST_LIBRARIES pthread)
target_link_libraries(test_taskqueue ${TEST_LIBRARIES})
