cmake_minimum_required(VERSION 2.6)
#FrostTools static library
PROJECT(FrostTools)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC ")

include_directories(${PROJECT_SOURCE_DIR}/include)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#ADD_LIBRARY(frosttools STATIC src/network.cpp)

include(CPack)

set(FROSTTOOLS_HEADERS 
	include/frosttools/lockfree.hpp 
	include/logger.hpp 
	include/modile.hpp 
	include/ringbuffer.hpp
	include/threads.hpp
	include/time.hpp
	include/delegate.hpp
	include/autoptr.hpp)

option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
	add_subdirectory(tests)
endif(BUILD_TESTS)


ADD_LIBRARY(frosttools src/network.cpp)

# install target
install(DIRECTORY include/frosttools DESTINATION include FILES_MATCHING PATTERN "*.h")
install(DIRECTORY include/frosttools DESTINATION include FILES_MATCHING PATTERN "*.hpp")
#install(DIRECTORY include/frosttools DESTINATION include FILES_MATCHING PATTERN "*.hxx")

install(TARGETS frosttools DESTINATION lib/frosttools)
install(FILES FrostToolsConfig.cmake DESTINATION lib/frosttools)
#INSTALL_TARGET(/lib frosttools)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
