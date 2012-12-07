cmake_minimum_required(VERSION 2.6)
#FrostTools static library
PROJECT(FrostTools)

include_directories(${PROJECT_SOURCE_DIR}/include)

#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#ADD_LIBRARY(frosttools STATIC src/network.cpp src/connection.cpp)
ADD_LIBRARY(frosttools SHARED src/network.cpp src/connection.cpp)

# install target
install(DIRECTORY include/frosttools DESTINATION include FILES_MATCHING PATTERN "*.h")
install(DIRECTORY include/frosttools DESTINATION include FILES_MATCHING PATTERN "*.hpp")
#install(DIRECTORY include/frosttools DESTINATION include FILES_MATCHING PATTERN "*.hxx")

install(TARGETS frosttools DESTINATION lib)
#INSTALL_TARGET(/lib frosttools)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
