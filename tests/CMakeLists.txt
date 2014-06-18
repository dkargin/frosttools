SET(CMAKE_CXX_FLAGS "-std=c++0x")


if(UNIX)
	set(TEST_LIBRARIES pthread)
endif(UNIX)

include_directories(../include)

#add_executable(test_taskqueue taskqueue.cpp)
#target_link_libraries(test_taskqueue ${TEST_LIBRARIES})

#add_executable(test_lockfree lockfree.cpp)
#add_executable(test_delegate testDelegate.cpp)
add_executable(test_math testMath.cpp)
add_executable(test_cond_var condition_variable.cpp )
target_link_libraries(test_cond_var ${TEST_LIBRARIES})
