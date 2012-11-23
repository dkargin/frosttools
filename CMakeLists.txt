#FrostTools static library

ADD_LIBRARY(frosttools network.cpp)

INSTALL_TARGET(/lib frosttools)
INSTALL_TARGET(/include/frosttools "\.hxx") 
