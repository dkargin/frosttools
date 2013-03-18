Library mostly consists of header-only code, but network.h requires static library

Frosttools building:

	$mkdir build
	$cd build
	$cmake ..
	$make
	#make install

Installs headers and library to /usr/local/include/frosttools, /usr/local/lib

Make sure you have specified path to shared library 
LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
