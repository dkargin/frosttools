# Try to find frosttools
# FROSTTOOLS_FOUND
# FROSTTOOLS_INCLUDE_DIRS
# FROSTTOOLS_LIBRARIES
# FROSTTOOLS_DEFINITIONS

find_path(FROSTTOOLS_INCLUDE_DIR frosttools/frosttools.h HINTS /usr/include /usr/local/include ${FrostTools_DIR}/include)

if(FROSTTOOLS_INCLUDE_DIR)
	message(STATUS "FrostTools found : ${FROSTTOOLS_INCLUDE_DIR}")
	set(FROSTTOOLS_FOUND TRUE)
else(FROSTTOOLS_INCLUDE_DIR)
	message(STATUS "FrostTools not found")
	set(FROSTTOOLS_FOUND FALSE)
endif(FROSTTOOLS_INCLUDE_DIR)

#mark_as_advanced(FROSTTOOLS_INCLUDE_DIR)