SET(CMAKE_CXX_FLAGS "-std=c++0x")

add_executable(test_taskqueue taskqueue.cpp)
if(UNIX)
set(TEST_LIBRARIES pthread)
endif(UNIX)
target_link_libraries(test_taskqueue ${TEST_LIBRARIES})
