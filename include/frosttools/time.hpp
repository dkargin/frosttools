#ifndef _FROSTTOOLS_TIMER_HPP_
#define _FROSTTOOLS_TIMER_HPP_

#ifdef WIN32
#include "win32/time_impl.hpp"
#else
#include "linux/time_impl.hpp"
#endif

#endif
