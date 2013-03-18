# Try to find frosttools
# FROSTTOOLS_FOUND
# FROSTTOOLS_INCLUDE_DIRS
# FROSTTOOLS_LIBRARIES
# FROSTTOOLS_DEFINITIONS

find_path(FROSTTOOLS_INCLUDE_DIR frosttools/frosttools.h HINTS /usr/include /usr/local/include ${FrostTools_DIR}/include)

find_library(FROSTTOOLS_LIBRARY NAMES frosttools HINTS /usr/lib /usr/local/lib ${FrostTools_DIR}/lib)

set(FROSTTOOLS_FOUND TRUE)

if(FROSTTOOLS_INCLUDE_DIR)
	message(STATUS "FrostTools found : ${FROSTTOOLS_INCLUDE_DIR}")
	set(FROSTTOOLS_INCLUDE_DIRS ${FROSTTOOLS_INCLUDE_DIRS})	
else(FROSTTOOLS_INCLUDE_DIR)
	message(STATUS "FrostTools not found")
	set(FROSTTOOLS_FOUND FALSE)
endif(FROSTTOOLS_INCLUDE_DIR)

if(FROSTTOOLS_LIBRARY)
	message(STATUS "FrostTools lib found : ${FROSTTOOLS_LIBRARY}")	
	set(FROSTTOOLS_LIBRARIES ${FROSTTOOLS_LIBRARY})
else(FROSTTOOLS_LIBRARY)
	message(STATUS "FrostTools library not found")
	set(FROSTTOOLS_FOUND FALSE)
endif(FROSTTOOLS_LIBRARY)

#mark_as_advanced(FROSTTOOLS_INCLUDE_DIR)