// Build flags. How do you want your server built?
#define DEBUG 0
#define ASSERT 1
#define MEMORY_DEBUGGING 0 //enable this to turn on really fancy debugging of memory leaks, etc...


#define PLATFORM_SERVER_BIN_NAME "AccessServer"
#define PLATFORM_SERVER_TEXT_NAME "Access Server"
#define MMAP_TABLES 0


// Platform-specific switches
#if __Win32__

#define USE_ATOMICLIB 0
#define MACOSXEVENTQUEUE 0
#define __PTHREADS__    0
#define __PTHREADS_MUTEXES__    0
//#define BIGENDIAN     0   // Defined equivalently inside windows
#define ALLOW_NON_WORD_ALIGN_ACCESS 1
#define USE_THREAD      0 //Flag used in QTProxy
#define THREADING_IS_COOPERATIVE        0
#define USE_THR_YIELD   0
#define kPlatformNameString     "Win32"
#define EXPORT  __declspec(dllexport)
#ifndef USE_DEFAULT_STD_LIB
    #define USE_DEFAULT_STD_LIB 1
#endif

#elif __linux__ 

#include <endian.h>
#if __BYTE_ORDER == BIG_ENDIAN
    #define BIGENDIAN      1
#else
    #define BIGENDIAN      0
#endif

#define USE_ATOMICLIB 0
#define MACOSXEVENTQUEUE 0
#define __PTHREADS__    1
#define __PTHREADS_MUTEXES__    1
#define ALLOW_NON_WORD_ALIGN_ACCESS 1
#define USE_THREAD      0 //Flag used in QTProxy
#define THREADING_IS_COOPERATIVE        0 
#define USE_THR_YIELD   0
#define kPlatformNameString     "Linux"
#define EXPORT
#define _REENTRANT 1

#elif __linuxppc__ 

#include <endian.h>
#if __BYTE_ORDER == BIG_ENDIAN
    #define BIGENDIAN      1
#else
    #define BIGENDIAN      0
#endif

#define USE_ATOMICLIB 0
#define MACOSXEVENTQUEUE 0
#define __PTHREADS__    1
#define __PTHREADS_MUTEXES__    1
#define ALLOW_NON_WORD_ALIGN_ACCESS 1
#define USE_THREAD      0 //Flag used in QTProxy
#define THREADING_IS_COOPERATIVE        0 
#define USE_THR_YIELD   0
#define kPlatformNameString     "LinuxPPC"
#define EXPORT
#define _REENTRANT 1

#elif __FreeBSD__ 

#include <machine/endian.h>
#if BYTE_ORDER == BIG_ENDIAN
    #define BIGENDIAN      1
#else
    #define BIGENDIAN      0
#endif

#define USE_ATOMICLIB 0
#define MACOSXEVENTQUEUE 0
#define __PTHREADS__    1
#define __PTHREADS_MUTEXES__    1
#define ALLOW_NON_WORD_ALIGN_ACCESS 1
#define USE_THREAD      1 //Flag used in QTProxy
#define THREADING_IS_COOPERATIVE        1 
#define USE_THR_YIELD   0
#define kPlatformNameString     "FreeBSD"
#define EXPORT
#define _REENTRANT 1

#elif __solaris__ 

#ifdef sparc
   #define BIGENDIAN 1
#endif
#ifdef _M_IX86
   #define BIGENDIAN 0
#endif
#ifdef _M_ALPHA
   #define BIGENDIAN 0
#endif
#ifndef BIGENDIAN
  #error NEED BIGENDIAN DEFINITION 0 OR 1 FOR PLATFORM
#endif

#define USE_ATOMICLIB 0
#define MACOSXEVENTQUEUE 0
#define __PTHREADS__    1
#define __PTHREADS_MUTEXES__    1
#define ALLOW_NON_WORD_ALIGN_ACCESS 0
#define USE_THREAD      1 //Flag used in QTProxy
#define THREADING_IS_COOPERATIVE        0
#define USE_THR_YIELD   0
#define kPlatformNameString     "Solaris"
#define EXPORT
#define _REENTRANT 1

#elif __sgi__ 

#define USE_ATOMICLIB 0
#define MACOSXEVENTQUEUE 0
#define __PTHREADS__    1
#define __PTHREADS_MUTEXES__    1
#define BIGENDIAN               1
#define ALLOW_NON_WORD_ALIGN_ACCESS 0
#define USE_THREAD              1 //Flag used in QTProxy
#define THREADING_IS_COOPERATIVE                0
#define USE_THR_YIELD   0
#define kPlatformNameString     "IRIX"
#define EXPORT
#define _REENTRANT 1

#elif __hpux__ 

#define USE_ATOMICLIB 0
#define MACOSXEVENTQUEUE 0
#define __PTHREADS__    1
#define __PTHREADS_MUTEXES__    1
#define BIGENDIAN               1
#define ALLOW_NON_WORD_ALIGN_ACCESS 0
#define USE_THREAD              1 //Flag used in QTProxy
#define THREADING_IS_COOPERATIVE                0
#define USE_THR_YIELD   0
#define kPlatformNameString     "HP-UX"
#define EXPORT
#define _REENTRANT 1

#elif defined(__osf__)

#define __osf__ 1
#define USE_ATOMICLIB 0
#define MACOSXEVENTQUEUE 0
#define __PTHREADS__    1
#define __PTHREADS_MUTEXES__    1
#define BIGENDIAN       0
#define ALLOW_NON_WORD_ALIGN_ACCESS 0
#define USE_THREAD      1 //Flag used in QTProxy
#define THREADING_IS_COOPERATIVE        0
#define USE_THR_YIELD   0
#define kPlatformNameString     "Tru64UNIX"
#define EXPORT

#endif
