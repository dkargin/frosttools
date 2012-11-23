#ifndef _MODULE_HPP_
#define _MODULE_HPP_

/// include platform-specific variants
#ifdef WIN32
#include "win32/module_impl.hpp"
#else
#include "linux/module_impl.hpp"
#endif

#endif