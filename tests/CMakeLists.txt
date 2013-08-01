SET(CMAKE_CXX_FLAGS "-std=c++0x")


if(UNIX)
set(TEST_LIBRARIES pthread)
endif(UNIX)

add_executable(test_taskqueue taskqueue.cpp)
target_link_libraries(test_taskqueue ${TEST_LIBRARIES})

add_executable(test_lockfree lockfree.cpp)
add_executable(test_delegate testDelegate.cpp)
add_executable(test_math testMath.cpp)
