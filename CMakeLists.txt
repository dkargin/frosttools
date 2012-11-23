#FrostTools static library

ADD_LIBRARY(frosttools network.cpp)

# install target
INSTALL_TARGET(/lib frosttools)
INSTALL_TARGET(/include/frosttools "\.hxx") 
INSTALL_TARGET(/include/frosttools "\.hpp") 
INSTALL_TARGET(/include/frosttools "\.h") 

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
