#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC ")

include_directories(${PROJECT_SOURCE_DIR}/include)

set(FROSTTOOLS_HEADERS 
	include/frosttools/lockfree.hpp 
	include/logger.hpp 
	include/modile.hpp 
	include/ringbuffer.hpp
	include/threads.hpp
	include/time.hpp
	include/delegate.hpp
	include/autoptr.hpp)

ADD_LIBRARY(frosttools SHARED network.cpp)

install(TARGETS frosttools
  # IMPORTANT: Add the FrostTools library to the "export-set"
  # EXPORT FrostToolsTargets
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}")