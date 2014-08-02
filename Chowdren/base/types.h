#ifndef CHOWDREN_TYPES
#define CHOWDREN_TYPES

#if defined(_MSC_VER)

// Define _W64 macros to mark types changing their size, like intptr_t.
#ifndef _W64
#  if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#     define _W64 __w64
#  else
#     define _W64
#  endif
#endif

#ifdef _WIN64
   typedef signed __int64    intptr_t;
   typedef unsigned __int64  uintptr_t;
#else
   typedef _W64 signed int   intptr_t;
   typedef _W64 unsigned int uintptr_t;
#endif

typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else

#include <stdint.h>

#endif

#if defined(CHOWDREN_IS_PS4) || defined(CHOWDREN_IS_VITA)
#include <unordered_map>
#define hash_map std::unordered_map
#else
#include <boost/unordered_map.hpp>
#define hash_map boost::unordered_map
#endif

#include <boost/container/vector.hpp>
using boost::container::vector;

#endif // CHOWDREN_TYPES
