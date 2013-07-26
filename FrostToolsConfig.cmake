# Try to find frosttools
# FROSTTOOLS_FOUND
# FROSTTOOLS_INCLUDE_DIRS
# FROSTTOOLS_LIBRARY_DIRS
# FROSTTOOLS_LIBRARIES
# FROSTTOOLS_DEFINITIONS

find_path(FROSTTOOLS_INCLUDE_DIR frosttools/frosttools.h HINTS /usr/include /usr/local/include ${FrostTools_DIR}/include)
find_library(FROSTTOOLS_LIBRARY_NET NAMES frosttools HINTS /usr/lib/frosttools/ /usr/local/lib/frosttools/ ${FrostTools_DIR}/lib)

if(FROSTTOOLS_INCLUDE_DIR)
	message(STATUS "FrostTools found : ${FROSTTOOLS_INCLUDE_DIR}")
	set(FROSTTOOLS_FOUND TRUE)
	if(UNIX)
		set(FROSTTOOLS_LIBRARIES ${FROSTTOOLS_LIBRARIES} pthread rt)
	endif(UNIX)
else(FROSTTOOLS_INCLUDE_DIR)
	message(STATUS "FrostTools not found")
	set(FROSTTOOLS_FOUND FALSE)
endif(FROSTTOOLS_INCLUDE_DIR)

if(FROSTTOOLS_LIBRARY_NET)
	set(FROSTTOOLS_LIBRARIES ${FROSTTOOLS_LIBRARIES} ${FROSTTOOLS_LIBRARY_NET})	
	message(STATUS "FrostTools lib found : ${FROSTTOOLS_LIBRARY_NET}")	
else(FROSTTOOLS_LIBRARY_NET)
	message(STATUS "FrostTools library not found")
	set(FROSTTOOLS_FOUND FALSE)
endif(FROSTTOOLS_LIBRARY_NET)

#mark_as_advanced(FROSTTOOLS_INCLUDE_DIR)
