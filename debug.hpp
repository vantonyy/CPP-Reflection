#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <cassert>

#define DEBUG_BUILD

#ifdef DEBUG_BUILD
#define ASSERT( x ) assert( x )
#else 
#define ASSERT ( x ) (void) x;
#endif // DEBUG_BUILD

#endif // DEBUG_HPP