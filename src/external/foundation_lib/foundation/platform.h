/* platform.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
 *
 * This library provides a cross-platform foundation library in C11 providing basic support data types and
 * functions to write applications and games in a platform-independent fashion. The latest source code is
 * always available at
 *
 * https://github.com/rampantpixels/foundation_lib
 *
 * This library is put in the public domain; you can redistribute it and/or modify it without any restrictions.
 *
 */

#pragma once

//Lint warning inhibitors
/*lint -e717    We use do {} while(0) constructs in macros deliberately */

#if !defined( FOUNDATION_COMPILE )
#  define FOUNDATION_COMPILE 0
#endif

#if FOUNDATION_COMPILE
#  ifdef __cplusplus
#  define FOUNDATION_EXTERN extern "C"
#  define FOUNDATION_API extern "C"
#  else
#  define FOUNDATION_EXTERN extern
#  define FOUNDATION_API extern
#  endif
#else
#  ifdef __cplusplus
#  define FOUNDATION_EXTERN extern "C"
#  define FOUNDATION_API extern "C"
#  else
#  define FOUNDATION_EXTERN extern
#  define FOUNDATION_API extern
#  endif
#endif

//Platforms
#define FOUNDATION_PLATFORM_ANDROID 0
#define FOUNDATION_PLATFORM_BSD 0
#define FOUNDATION_PLATFORM_IOS 0
#define FOUNDATION_PLATFORM_IOS_SIMULATOR 0
#define FOUNDATION_PLATFORM_LINUX 0
#define FOUNDATION_PLATFORM_LINUX_RASPBERRYPI 0
#define FOUNDATION_PLATFORM_MACOSX 0
#define FOUNDATION_PLATFORM_WINDOWS 0
#define FOUNDATION_PLATFORM_PNACL 0
#define FOUNDATION_PLATFORM_TIZEN 0

//Platform traits and groups
#define FOUNDATION_PLATFORM_APPLE 0
#define FOUNDATION_PLATFORM_POSIX 0

#define FOUNDATION_PLATFORM_FAMILY_MOBILE 0
#define FOUNDATION_PLATFORM_FAMILY_DESKTOP 0
#define FOUNDATION_PLATFORM_FAMILY_CONSOLE 0

//Architectures
#define FOUNDATION_ARCH_ARM 0
#define FOUNDATION_ARCH_ARM5 0
#define FOUNDATION_ARCH_ARM6 0
#define FOUNDATION_ARCH_ARM7 0
#define FOUNDATION_ARCH_ARM8 0
#define FOUNDATION_ARCH_ARM_64 0
#define FOUNDATION_ARCH_ARM8_64 0
#define FOUNDATION_ARCH_X86 0
#define FOUNDATION_ARCH_X86_64 0
#define FOUNDATION_ARCH_PPC 0
#define FOUNDATION_ARCH_PPC_64 0
#define FOUNDATION_ARCH_IA64 0
#define FOUNDATION_ARCH_MIPS 0
#define FOUNDATION_ARCH_MIPS_64 0
#define FOUNDATION_ARCH_GENERIC 0

//Architecture details
#define FOUNDATION_ARCH_SSE2 0
#define FOUNDATION_ARCH_SSE3 0
#define FOUNDATION_ARCH_SSE4 0
#define FOUNDATION_ARCH_SSE4_FMA3 0
#define FOUNDATION_ARCH_NEON 0
#define FOUNDATION_ARCH_THUMB 0

#define FOUNDATION_ARCH_ENDIAN_LITTLE 0
#define FOUNDATION_ARCH_ENDIAN_BIG 0

//Compilers
#define FOUNDATION_COMPILER_CLANG 0
#define FOUNDATION_COMPILER_GCC 0
#define FOUNDATION_COMPILER_MSVC 0
#define FOUNDATION_COMPILER_INTEL 0


//First, platforms and architectures

#if defined( __pnacl__ )

#  undef  FOUNDATION_PLATFORM_PNACL
#  define FOUNDATION_PLATFORM_PNACL 1

#  define FOUNDATION_PLATFORM_NAME "PNaCl"
#  define FOUNDATION_PLATFORM_DESCRIPTION "PNaCl"

#  undef  FOUNDATION_ARCH_GENERIC
#  define FOUNDATION_ARCH_GENERIC 1

#  undef  FOUNDATION_ARCH_ENDIAN_LITTLE
#  define FOUNDATION_ARCH_ENDIAN_LITTLE 1

#  ifdef __STRICT_ANSI__
#    undef __STRICT_ANSI__
#  endif

// Android
#elif defined( __ANDROID__ )

#  undef  FOUNDATION_PLATFORM_ANDROID
#  define FOUNDATION_PLATFORM_ANDROID 1

// Compatibile platforms
#  undef  FOUNDATION_PLATFORM_POSIX
#  define FOUNDATION_PLATFORM_POSIX 1

#  define FOUNDATION_PLATFORM_NAME "Android"

// Architecture and detailed description
#  if defined( __arm__ )
#    undef  FOUNDATION_ARCH_ARM
#    define FOUNDATION_ARCH_ARM 1
#    ifdef __ARM_ARCH_7A__
#      undef  FOUNDATION_ARCH_ARM7
#      define FOUNDATION_ARCH_ARM7 1
#      define FOUNDATION_PLATFORM_DESCRIPTION "Android ARMv7"
#    elif defined(__ARM_ARCH_5TE__)
#      undef  FOUNDATION_ARCH_ARM5
#      define FOUNDATION_ARCH_ARM5 1
#      define FOUNDATION_PLATFORM_DESCRIPTION "Android ARMv5"
#    else
#      error Unsupported ARM architecture
#    endif
#  elif defined( __aarch64__ )
#    undef  FOUNDATION_ARCH_ARM
#    define FOUNDATION_ARCH_ARM 1
#    undef  FOUNDATION_ARCH_ARM_64
#    define FOUNDATION_ARCH_ARM_64 1
//Assume ARMv8 for now
//#    if defined( __ARM_ARCH ) && ( __ARM_ARCH == 8 )
#      undef  FOUNDATION_ARCH_ARM8_64
#      define FOUNDATION_ARCH_ARM8_64 1
#      define FOUNDATION_PLATFORM_DESCRIPTION "Android ARM64v8"
//#    else
//#      error Unrecognized AArch64 architecture
//#    endif
#  elif defined( __i386__ )
#    undef  FOUNDATION_ARCH_X86
#    define FOUNDATION_ARCH_X86 1
#    define FOUNDATION_PLATFORM_DESCRIPTION "Android x86"
#  elif defined( __x86_64__ )
#    undef  FOUNDATION_ARCH_X86_64
#    define FOUNDATION_ARCH_X86_64 1
#    define FOUNDATION_PLATFORM_DESCRIPTION "Android x86-64"
#  elif ( defined( __mips__ ) && defined( __mips64 ) )
#    undef  FOUNDATION_ARCH_MIPS
#    define FOUNDATION_ARCH_MIPS 1
#    undef  FOUNDATION_ARCH_MIPS_64
#    define FOUNDATION_ARCH_MIPS_64 1
#    define FOUNDATION_PLATFORM_DESCRIPTION "Android MIPS64"
#    ifndef _MIPS_ISA
#      define _MIPS_ISA 7 /*_MIPS_ISA_MIPS64*/
#    endif
#  elif defined( __mips__ )
#    undef  FOUNDATION_ARCH_MIPS
#    define FOUNDATION_ARCH_MIPS 1
#    define FOUNDATION_PLATFORM_DESCRIPTION "Android MIPS"
#    ifndef _MIPS_ISA
#      define _MIPS_ISA 6 /*_MIPS_ISA_MIPS32*/
#    endif
#  else
#    error Unknown architecture
#  endif

// Traits
#  if FOUNDATION_ARCH_MIPS
#    if defined( __MIPSEL__ ) || defined( __MIPSEL ) || defined( _MIPSEL )
#      undef  FOUNDATION_ARCH_ENDIAN_LITTLE
#      define FOUNDATION_ARCH_ENDIAN_LITTLE 1
#    else
#      undef  FOUNDATION_ARCH_ENDIAN_BIG
#      define FOUNDATION_ARCH_ENDIAN_BIG 1
#    endif
#  elif defined( __AARCH64EB__ ) || defined( __ARMEB__ )
#    undef  FOUNDATION_ARCH_ENDIAN_BIG
#    define FOUNDATION_ARCH_ENDIAN_BIG 1
#  else
#    undef  FOUNDATION_ARCH_ENDIAN_LITTLE
#    define FOUNDATION_ARCH_ENDIAN_LITTLE 1
#  endif

#  undef  FOUNDATION_PLATFORM_FAMILY_MOBILE
#  define FOUNDATION_PLATFORM_FAMILY_MOBILE 1

#  undef  FOUNDATION_PLATFORM_FAMILY_CONSOLE
#  define FOUNDATION_PLATFORM_FAMILY_CONSOLE 1

// Tizen
#elif defined( __TIZEN__ )

#  undef  FOUNDATION_PLATFORM_TIZEN
#  define FOUNDATION_PLATFORM_TIZEN 1

// Compatibile platforms
#  undef  FOUNDATION_PLATFORM_POSIX
#  define FOUNDATION_PLATFORM_POSIX 1

#  define FOUNDATION_PLATFORM_NAME "Tizen"

// Architecture and detailed description
#  if defined( __arm__ )
#    undef  FOUNDATION_ARCH_ARM
#    define FOUNDATION_ARCH_ARM 1
#    ifdef __ARM_ARCH_7A__
#      undef  FOUNDATION_ARCH_ARM7
#      define FOUNDATION_ARCH_ARM7 1
#      define FOUNDATION_PLATFORM_DESCRIPTION "Tizen ARMv7"
#    elif defined(__ARM_ARCH_5TE__)
#      undef  FOUNDATION_ARCH_ARM5
#      define FOUNDATION_ARCH_ARM5 1
#      define FOUNDATION_PLATFORM_DESCRIPTION "Tizen ARMv5"
#    else
#      error Unsupported ARM architecture
#    endif
#  elif defined( __i386__ )
#    undef  FOUNDATION_ARCH_X86
#    define FOUNDATION_ARCH_X86 1
#    define FOUNDATION_PLATFORM_DESCRIPTION "Tizen x86"
#  elif defined( __x86_64__ )
#    undef  FOUNDATION_ARCH_X86_64
#    define FOUNDATION_ARCH_X86_64 1
#    define FOUNDATION_PLATFORM_DESCRIPTION "Tizen x86-64"
#  else
#    error Unknown architecture
#  endif

// Traits
#  if defined( __AARCH64EB__ ) || defined( __ARMEB__ )
#    undef  FOUNDATION_ARCH_ENDIAN_BIG
#    define FOUNDATION_ARCH_ENDIAN_BIG 1
#  else
#    undef  FOUNDATION_ARCH_ENDIAN_LITTLE
#    define FOUNDATION_ARCH_ENDIAN_LITTLE 1
#  endif

#  undef  FOUNDATION_PLATFORM_FAMILY_MOBILE
#  define FOUNDATION_PLATFORM_FAMILY_MOBILE 1

#  undef  FOUNDATION_PLATFORM_FAMILY_CONSOLE
#  define FOUNDATION_PLATFORM_FAMILY_CONSOLE 1

// MacOS X and iOS
#elif ( defined( __APPLE__ ) && __APPLE__ )

#  undef  FOUNDATION_PLATFORM_APPLE
#  define FOUNDATION_PLATFORM_APPLE 1

#  undef  FOUNDATION_PLATFORM_POSIX
#  define FOUNDATION_PLATFORM_POSIX 1

#  include <TargetConditionals.h>

#  if defined( __IPHONE__ ) || ( defined( TARGET_OS_IPHONE ) && TARGET_OS_IPHONE ) || ( defined( TARGET_IPHONE_SIMULATOR ) && TARGET_IPHONE_SIMULATOR )

#    undef  FOUNDATION_PLATFORM_IOS
#    define FOUNDATION_PLATFORM_IOS 1

#    define FOUNDATION_PLATFORM_NAME "iOS"

#    if defined( __arm__ )
#      undef  FOUNDATION_ARCH_ARM
#      define FOUNDATION_ARCH_ARM 1
#      if defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7S__)
#        undef  FOUNDATION_ARCH_ARM7
#        define FOUNDATION_ARCH_ARM7 1
#        define FOUNDATION_PLATFORM_DESCRIPTION "iOS ARMv7"
#        ifndef __ARM_NEON__
#          error Missing ARM NEON support
#        endif
#      elif defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6__)
#        undef  FOUNDATION_ARCH_ARM6
#        define FOUNDATION_ARCH_ARM6 1
#        define FOUNDATION_PLATFORM_DESCRIPTION "iOS ARMv6"
#      else
#        error Unrecognized ARM architecture
#      endif
#    elif defined( __arm64__ )
#      undef  FOUNDATION_ARCH_ARM
#      define FOUNDATION_ARCH_ARM 1
#      undef  FOUNDATION_ARCH_ARM_64
#      define FOUNDATION_ARCH_ARM_64 1
#      if defined( __ARM64_ARCH_8__ )
#        undef  FOUNDATION_ARCH_ARM8_64
#        define FOUNDATION_ARCH_ARM8_64 1
#        define FOUNDATION_PLATFORM_DESCRIPTION "iOS ARM64v8"
#      else
#        error Unrecognized ARM architecture
#      endif
#    elif defined( __i386__ )
#      undef  FOUNDATION_PLATFORM_IOS_SIMULATOR
#      define FOUNDATION_PLATFORM_IOS_SIMULATOR 1
#      undef  FOUNDATION_ARCH_X86
#      define FOUNDATION_ARCH_X86 1
#      define FOUNDATION_PLATFORM_DESCRIPTION "iOS x86 (simulator)"
#    elif defined( __x86_64__ )
#      undef  FOUNDATION_PLATFORM_IOS_SIMULATOR
#      define FOUNDATION_PLATFORM_IOS_SIMULATOR 1
#      undef  FOUNDATION_ARCH_X86_64
#      define FOUNDATION_ARCH_X86_64 1
#      define FOUNDATION_PLATFORM_DESCRIPTION "iOS x86_64 (simulator)"
#    else
#      error Unknown architecture
#    endif

#    undef  FOUNDATION_ARCH_ENDIAN_LITTLE
#    define FOUNDATION_ARCH_ENDIAN_LITTLE 1

#    undef  FOUNDATION_PLATFORM_FAMILY_MOBILE
#    define FOUNDATION_PLATFORM_FAMILY_MOBILE 1

#    undef  FOUNDATION_PLATFORM_FAMILY_CONSOLE
#    define FOUNDATION_PLATFORM_FAMILY_CONSOLE 1

#  elif defined( __MACH__ )

#    undef  FOUNDATION_PLATFORM_MACOSX
#    define FOUNDATION_PLATFORM_MACOSX 1

#    define FOUNDATION_PLATFORM_NAME "MacOSX"

#    if defined( __x86_64__ ) ||  defined( __x86_64 ) || defined( __amd64 )
#      undef  FOUNDATION_ARCH_X86_64
#      define FOUNDATION_ARCH_X86_64 1
#      undef  FOUNDATION_ARCH_ENDIAN_LITTLE
#      define FOUNDATION_ARCH_ENDIAN_LITTLE 1
#      define FOUNDATION_PLATFORM_DESCRIPTION "MacOSX x86-64"
#    elif defined( __i386__ ) || defined( __intel__ )
#      undef  FOUNDATION_ARCH_X86
#      define FOUNDATION_ARCH_X86 1
#      undef  FOUNDATION_ARCH_ENDIAN_LITTLE
#      define FOUNDATION_ARCH_ENDIAN_LITTLE 1
#      define FOUNDATION_PLATFORM_DESCRIPTION "MacOSX x86"

#    elif defined( __powerpc64__ ) || defined( __POWERPC64__ )
#      undef  FOUNDATION_ARCH_PPC_64
#      define FOUNDATION_ARCH_PPC_64 1
#      undef  FOUNDATION_ARCH_ENDIAN_BIG
#      define FOUNDATION_ARCH_ENDIAN_BIG 1
#      define FOUNDATION_PLATFORM_DESCRIPTION "MacOSX PPC64"
#    elif defined( __powerpc__ ) || defined( __POWERPC__ )
#      undef  FOUNDATION_ARCH_PPC
#      define FOUNDATION_ARCH_PPC 1
#      undef  FOUNDATION_ARCH_ENDIAN_BIG
#      define FOUNDATION_ARCH_ENDIAN_BIG 1
#      define FOUNDATION_PLATFORM_DESCRIPTION "MacOSX PPC"

#    else
#      error Unknown architecture
#    endif

#    undef  FOUNDATION_PLATFORM_FAMILY_DESKTOP
#    define FOUNDATION_PLATFORM_FAMILY_DESKTOP 1

#  else
#    error Unknown Apple Platform
#  endif

// Linux
#elif ( defined( __linux__ ) || defined( __linux ) )

#  undef  FOUNDATION_PLATFORM_LINUX
#  define FOUNDATION_PLATFORM_LINUX 1

#  undef  FOUNDATION_PLATFORM_POSIX
#  define FOUNDATION_PLATFORM_POSIX 1

#  define FOUNDATION_PLATFORM_NAME "Linux"

#  if defined( __x86_64__ ) || defined( __x86_64 ) || defined( __amd64 )
#    undef  FOUNDATION_ARCH_X86_64
#    define FOUNDATION_ARCH_X86_64 1
#    undef  FOUNDATION_ARCH_ENDIAN_LITTLE
#    define FOUNDATION_ARCH_ENDIAN_LITTLE 1
#    define FOUNDATION_PLATFORM_DESCRIPTION "Linux x86-64"
#  elif defined( __i386__ ) || defined( __intel__ ) || defined( _M_IX86 )
#    undef  FOUNDATION_ARCH_X86
#    define FOUNDATION_ARCH_X86 1
#    undef  FOUNDATION_ARCH_ENDIAN_LITTLE
#    define FOUNDATION_ARCH_ENDIAN_LITTLE 1
#    define FOUNDATION_PLATFORM_DESCRIPTION "Linux x86"

#  elif defined( __powerpc64__ ) || defined( __POWERPC64__ )
#    undef  FOUNDATION_ARCH_PPC_64
#    define FOUNDATION_ARCH_PPC_64 1
#    undef  FOUNDATION_ARCH_ENDIAN_BIG
#    define FOUNDATION_ARCH_ENDIAN_BIG 1
#    define FOUNDATION_PLATFORM_DESCRIPTION "Linux PPC64"
#  elif defined( __powerpc__ ) || defined( __POWERPC__ )
#    undef  FOUNDATION_ARCH_PPC
#    define FOUNDATION_ARCH_PPC 1
#    undef  FOUNDATION_ARCH_ENDIAN_BIG
#    define FOUNDATION_ARCH_ENDIAN_BIG 1
#    define FOUNDATION_PLATFORM_DESCRIPTION "Linux PPC"
#  elif defined( __arm__ )
#    undef  FOUNDATION_ARCH_ARM
#    define FOUNDATION_ARCH_ARM 1
#    if defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7S__)
#      undef  FOUNDATION_ARCH_ARM7
#      define FOUNDATION_ARCH_ARM7 1
#      define FOUNDATION_PLATFORM_DESCRIPTION "Linux ARMv7"
#      ifndef __ARM_NEON__
#        error Missing ARM NEON support
#      endif
#    elif defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6ZK__)
#      undef  FOUNDATION_ARCH_ARM6
#      define FOUNDATION_ARCH_ARM6 1
#      define FOUNDATION_PLATFORM_DESCRIPTION "Linux ARMv6"
#    else
#      error Unrecognized ARM architecture
#    endif

// Traits
#    if defined( __ARMEB__ )
#      undef  FOUNDATION_ARCH_ENDIAN_BIG
#      define FOUNDATION_ARCH_ENDIAN_BIG 1
#    else
#      undef  FOUNDATION_ARCH_ENDIAN_LITTLE
#      define FOUNDATION_ARCH_ENDIAN_LITTLE 1
#    endif

#  elif defined( __arm64__ ) || defined( __aarch64__ )
#    undef  FOUNDATION_ARCH_ARM
#    define FOUNDATION_ARCH_ARM 1
#    undef  FOUNDATION_ARCH_ARM_64
#    define FOUNDATION_ARCH_ARM_64 1
#    undef  FOUNDATION_ARCH_ENDIAN_LITTLE
#    define FOUNDATION_ARCH_ENDIAN_LITTLE 1
#    if defined( __ARM64_ARCH_8__ )
#      undef  FOUNDATION_ARCH_ARM8_64
#      define FOUNDATION_ARCH_ARM8_64 1
#      define FOUNDATION_PLATFORM_DESCRIPTION "Linux ARM64v8"
#    else
#      error Unrecognized ARM architecture
#    endif

// Traits
#    if defined( __AARCH64EB__ )
#      undef  FOUNDATION_ARCH_ENDIAN_BIG
#      define FOUNDATION_ARCH_ENDIAN_BIG 1
#    else
#      undef  FOUNDATION_ARCH_ENDIAN_LITTLE
#      define FOUNDATION_ARCH_ENDIAN_LITTLE 1
#    endif

#  else
#    error Unknown architecture
#  endif

#  if defined( __raspberrypi__ )
#    undef  FOUNDATION_PLATFORM_LINUX_RASPBERRYPI
#    define FOUNDATION_PLATFORM_LINUX_RASPBERRYPI 1
#  endif

#  undef  FOUNDATION_PLATFORM_FAMILY_DESKTOP
#  define FOUNDATION_PLATFORM_FAMILY_DESKTOP 1

#  ifndef _GNU_SOURCE
#    define _GNU_SOURCE
#  endif

//BSD family
#elif ( defined( __BSD__ ) || defined( __FreeBSD__ ) )

#  undef  FOUNDATION_PLATFORM_BSD
#  define FOUNDATION_PLATFORM_BSD 1

#  undef  FOUNDATION_PLATFORM_POSIX
#  define FOUNDATION_PLATFORM_POSIX 1

#  define FOUNDATION_PLATFORM_NAME "BSD"

#  if defined( __x86_64__ ) || defined( __x86_64 ) || defined( __amd64 )
#    undef  FOUNDATION_ARCH_X86_64
#    define FOUNDATION_ARCH_X86_64 1
#    undef  FOUNDATION_ARCH_ENDIAN_LITTLE
#    define FOUNDATION_ARCH_ENDIAN_LITTLE 1
#    define FOUNDATION_PLATFORM_DESCRIPTION "BSD x86-64"
#  elif defined( __i386__ ) || defined( __intel__ ) || defined( _M_IX86 )
#    undef  FOUNDATION_ARCH_X86
#    define FOUNDATION_ARCH_X86 1
#    undef  FOUNDATION_ARCH_ENDIAN_LITTLE
#    define FOUNDATION_ARCH_ENDIAN_LITTLE 1
#    define FOUNDATION_PLATFORM_DESCRIPTION "BSD x86"

#  elif defined( __powerpc64__ ) || defined( __POWERPC64__ )
#    undef  FOUNDATION_ARCH_PPC_64
#    define FOUNDATION_ARCH_PPC_64 1
#    undef  FOUNDATION_ARCH_ENDIAN_BIG
#    define FOUNDATION_ARCH_ENDIAN_BIG 1
#    define FOUNDATION_PLATFORM_DESCRIPTION "BSD PPC64"
#  elif defined( __powerpc__ ) || defined( __POWERPC__ )
#    undef  FOUNDATION_ARCH_PPC
#    define FOUNDATION_ARCH_PPC 1
#    undef  FOUNDATION_ARCH_ENDIAN_BIG
#    define FOUNDATION_ARCH_ENDIAN_BIG 1
#    define FOUNDATION_PLATFORM_DESCRIPTION "BSD PPC"

#  else
#    error Unknown architecture
#  endif

#  undef  FOUNDATION_PLATFORM_FAMILY_DESKTOP
#  define FOUNDATION_PLATFORM_FAMILY_DESKTOP 1

// Windows
#elif defined( _WIN32 ) || defined( __WIN32__ ) || defined( _WIN64 )

#  undef  FOUNDATION_PLATFORM_WINDOWS
#  define FOUNDATION_PLATFORM_WINDOWS 1

#  define FOUNDATION_PLATFORM_NAME "Windows"

#  if defined( __x86_64__ ) || defined( _M_AMD64 ) || defined( _AMD64_ )
#    undef  FOUNDATION_ARCH_X86_64
#    define FOUNDATION_ARCH_X86_64 1
#    define FOUNDATION_PLATFORM_DESCRIPTION "Windows x86-64"
#  elif defined( __x86__ ) || defined( _M_IX86 ) || defined( _X86_ )
#    undef  FOUNDATION_ARCH_X86
#    define FOUNDATION_ARCH_X86 1
#    define FOUNDATION_PLATFORM_DESCRIPTION "Windows x86"
#  elif defined( __ia64__ ) || defined( _M_IA64 ) || defined( _IA64_ )
#    undef  FOUNDATION_ARCH_IA64
#    define FOUNDATION_ARCH_IA64 1
#    define FOUNDATION_PLATFORM_DESCRIPTION "Windows IA-64"

#  else
#    error Unknown architecture
#  endif

#  undef  FOUNDATION_ARCH_ENDIAN_LITTLE
#  define FOUNDATION_ARCH_ENDIAN_LITTLE 1

#  undef  FOUNDATION_PLATFORM_FAMILY_DESKTOP
#  define FOUNDATION_PLATFORM_FAMILY_DESKTOP 1

#  if defined( FOUNDATION_COMPILE ) && FOUNDATION_COMPILE && !defined( _CRT_SECURE_NO_WARNINGS )
#    define _CRT_SECURE_NO_WARNINGS 1
#  endif

#else
#  error Unknown platform
#endif


//Utility macros
#define FOUNDATION_PREPROCESSOR_TOSTRING( x )          _FOUNDATION_PREPROCESSOR_TOSTRING(x)
#define _FOUNDATION_PREPROCESSOR_TOSTRING( x )         #x

#define FOUNDATION_PREPROCESSOR_JOIN( a, b )           _FOUNDATION_PREPROCESSOR_JOIN( a, b )
#define _FOUNDATION_PREPROCESSOR_JOIN( a, b )          _FOUNDATION_PREPROCESSOR_JOIN_INTERNAL( a, b )
#define _FOUNDATION_PREPROCESSOR_JOIN_INTERNAL( a, b ) a##b

#define FOUNDATION_PREPROCESSOR_NARGS( ... )           _FOUNDATION_PREPROCESSOR_NARGS( __VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 )
#define _FOUNDATION_PREPROCESSOR_NARGS( _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _, ... ) _


//Architecture details
#ifdef __SSE2__
#  undef  FOUNDATION_ARCH_SSE2
#  define FOUNDATION_ARCH_SSE2 1
#endif

#ifdef __SSE3__
#  undef  FOUNDATION_ARCH_SSE3
#  define FOUNDATION_ARCH_SSE3 1
#endif

#ifdef __SSE4_1__
#  undef  FOUNDATION_ARCH_SSE4
#  define FOUNDATION_ARCH_SSE4 1
#endif

#ifdef __ARM_NEON__
#  undef  FOUNDATION_ARCH_NEON
#  define FOUNDATION_ARCH_NEON 1
#endif

#ifdef __thumb__
#  undef  FOUNDATION_ARCH_THUMB
#  define FOUNDATION_ARCH_THUMB 1
#endif


//Compilers

// CLang
#if defined( __clang__ )

#  undef  FOUNDATION_COMPILER_CLANG
#  define FOUNDATION_COMPILER_CLANG 1

#  define FOUNDATION_COMPILER_NAME "clang"
#  define FOUNDATION_COMPILER_DESCRIPTION FOUNDATION_COMPILER_NAME " " FOUNDATION_PREPROCESSOR_TOSTRING( __clang_major__ ) "." FOUNDATION_PREPROCESSOR_TOSTRING( __clang_minor__ )

#  define FOUNDATION_RESTRICT __restrict
#  if FOUNDATION_PLATFORM_WINDOWS
#    define FOUNDATION_THREADLOCAL
#  else
#    define FOUNDATION_THREADLOCAL __thread
#  endif

#  define FOUNDATION_ATTRIBUTE(x) __attribute__((__##x##__))
#  define FOUNDATION_ATTRIBUTE2(x,y) __attribute__((__##x##__(y)))
#  define FOUNDATION_ATTRIBUTE3(x,y,z) __attribute__((__##x##__(y,z)))

#  define FOUNDATION_DEPRECATED FOUNDATION_ATTRIBUTE( deprecated )
#  define FOUNDATION_FORCEINLINE inline FOUNDATION_ATTRIBUTE( always_inline )
#  define FOUNDATION_NOINLINE FOUNDATION_ATTRIBUTE( noinline )
#  define FOUNDATION_PURECALL FOUNDATION_ATTRIBUTE( pure )
#  define FOUNDATION_CONSTCALL FOUNDATION_ATTRIBUTE( const )
#  define FOUNDATION_ALIGN( alignment ) FOUNDATION_ATTRIBUTE2( aligned, alignment )
#  define FOUNDATION_ALIGNOF( type ) __alignof__( type )
#  define FOUNDATION_ALIGNED_STRUCT( name, alignment ) struct __attribute__((__aligned__(alignment))) name

#  if FOUNDATION_PLATFORM_WINDOWS
#    define STDCALL
#    ifndef __USE_MINGW_ANSI_STDIO
#      define __USE_MINGW_ANSI_STDIO 1
#    endif
#    ifndef _CRT_SECURE_NO_WARNINGS
#      define _CRT_SECURE_NO_WARNINGS 1
#    endif
#    ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#      define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 0
#    endif
#    ifndef _MSC_VER
#      define _MSC_VER 1300
#    endif
#    define USE_NO_MINGW_SETJMP_TWO_ARGS 1
#  endif

#  include <stdbool.h>
#  include <stdarg.h>

#  include <wchar.h>

// GCC
#elif defined( __GNUC__ )

#  undef  FOUNDATION_COMPILER_GCC
#  define FOUNDATION_COMPILER_GCC 1

#  define FOUNDATION_COMPILER_NAME "gcc"
#  define FOUNDATION_COMPILER_DESCRIPTION FOUNDATION_COMPILER_NAME " " FOUNDATION_PREPROCESSOR_TOSTRING( __GNUC__ ) "." FOUNDATION_PREPROCESSOR_TOSTRING( __GNUC_MINOR__ )

#  define FOUNDATION_RESTRICT __restrict
#  define FOUNDATION_THREADLOCAL __thread

#  define FOUNDATION_ATTRIBUTE(x) __attribute__((__##x##__))
#  define FOUNDATION_ATTRIBUTE2(x,y) __attribute__((__##x##__(y)))
#  define FOUNDATION_ATTRIBUTE3(x,y,z) __attribute__((__##x##__(y,z)))

#  define FOUNDATION_DEPRECATED FOUNDATION_ATTRIBUTE( deprecated )
#  define FOUNDATION_FORCEINLINE inline FOUNDATION_ATTRIBUTE( always_inline )
#  define FOUNDATION_NOINLINE FOUNDATION_ATTRIBUTE( noinline )
#  define FOUNDATION_PURECALL FOUNDATION_ATTRIBUTE( pure )
#  define FOUNDATION_CONSTCALL FOUNDATION_ATTRIBUTE( const )
#  define FOUNDATION_ALIGN( alignment ) FOUNDATION_ATTRIBUTE2( aligned, alignment )
#  define FOUNDATION_ALIGNOF( type ) __alignof__( type )
#  define FOUNDATION_ALIGNED_STRUCT( name, alignment ) struct FOUNDATION_ALIGN( alignment ) name

#  if FOUNDATION_PLATFORM_WINDOWS
#    define STDCALL
#    ifndef __USE_MINGW_ANSI_STDIO
#      define __USE_MINGW_ANSI_STDIO 1
#    endif
#    ifndef _CRT_SECURE_NO_WARNINGS
#      define _CRT_SECURE_NO_WARNINGS 1
#    endif
#    ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#      define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 0
#    endif
#    ifndef _MSC_VER
#      define _MSC_VER 1300
#    endif
#  endif

#  include <stdbool.h>
#  include <stdarg.h>

#  include <wchar.h>

// Intel
#elif defined( __ICL ) || defined( __ICC ) || defined( __INTEL_COMPILER )

#  undef  FOUNDATION_COMPILER_INTEL
#  define FOUNDATION_COMPILER_INTEL 1

#  define FOUNDATION_COMPILER_NAME "intel"
#  if defined( __ICL )
#    define FOUNDATION_COMPILER_DESCRIPTION FOUNDATION_COMPILER_NAME " " FOUNDATION_PREPROCESSOR_TOSTRING( __ICL )
#  elif defined( __ICC )
#    define FOUNDATION_COMPILER_DESCRIPTION FOUNDATION_COMPILER_NAME " " FOUNDATION_PREPROCESSOR_TOSTRING( __ICC )
#  endif

#  define FOUNDATION_RESTRICT __restrict
#  define FOUNDATION_THREADLOCAL __declspec( thread )

#  define FOUNDATION_ATTRIBUTE(x)
#  define FOUNDATION_ATTRIBUTE2(x,y)
#  define FOUNDATION_ATTRIBUTE3(x,y,z)

#  define FOUNDATION_DEPRECATED
#  define FOUNDATION_FORCEINLINE __forceinline
#  define FOUNDATION_NOINLINE __declspec( noinline )
#  define FOUNDATION_PURECALL
#  define FOUNDATION_CONSTCALL
#  define FOUNDATION_ALIGN( alignment ) __declspec( align( alignment ) )
#  define FOUNDATION_ALIGNOF( type ) __alignof( type )
#  define FOUNDATION_ALIGNED_STRUCT( name, alignment ) FOUNDATION_ALIGN( alignment ) struct name

#  if FOUNDATION_PLATFORM_WINDOWS
#    define STDCALL __stdcall
#    define va_copy(d,s) ((d)=(s))
#  endif

#  include <intrin.h>

#  define bool _Bool
#  define true 1
#  define false 0
#  define __bool_true_false_are_defined 1

// Microsoft
#elif defined( _MSC_VER )

#  undef  FOUNDATION_COMPILER_MSVC
#  define FOUNDATION_COMPILER_MSVC 1

#  define FOUNDATION_COMPILER_NAME "msvc"
#  define FOUNDATION_COMPILER_DESCRIPTION FOUNDATION_COMPILER_NAME " " FOUNDATION_PREPROCESSOR_TOSTRING( _MSC_VER )

#  define FOUNDATION_ATTRIBUTE(x)
#  define FOUNDATION_ATTRIBUTE2(x,y)
#  define FOUNDATION_ATTRIBUTE3(x,y,z)

#  define FOUNDATION_RESTRICT __restrict
#  define FOUNDATION_THREADLOCAL __declspec( thread )

#  define FOUNDATION_DEPRECATED __declspec( deprecated )
#  define FOUNDATION_FORCEINLINE __forceinline
#  define FOUNDATION_NOINLINE __declspec( noinline )
#  define FOUNDATION_PURECALL
#  define FOUNDATION_CONSTCALL
#  define FOUNDATION_ALIGN( alignment ) __declspec( align( alignment ) )
#  define FOUNDATION_ALIGNOF( type ) __alignof( type )
#  define FOUNDATION_ALIGNED_STRUCT( name, alignment ) FOUNDATION_ALIGN( alignment ) struct name

#  pragma warning( disable : 4200 )

#  if FOUNDATION_PLATFORM_WINDOWS
#    define STDCALL __stdcall
#  endif

#  ifndef __cplusplus
typedef enum
{
	false = 0,
	true  = 1
} bool;
#  endif

#if _MSC_VER < 1800
#  define va_copy(d,s) ((d)=(s))
#endif

#  include <intrin.h>

#else

#  warning Unknown compiler

#  define FOUNDATION_COMPILER_NAME "unknown"
#  define FOUNDATION_COMPILER_DESCRIPTION "unknown"

#  define FOUNDATION_RESTRICT
#  define FOUNDATION_THREADLOCAL

#  define FOUNDATION_DEPRECATED
#  define FOUNDATION_FORCEINLINE
#  define FOUNDATION_NOINLINE
#  define FOUNDATION_PURECALL
#  define FOUNDATION_CONSTCALL
#  define FOUNDATION_ALIGN
#  define FOUNDATION_ALIGNOF
#  define FOUNDATION_ALIGNED_STRUCT( name, alignment ) struct name

typedef enum
{
  false = 0,
  true  = 1
} bool;

#endif

//Base data types
#include <stdint.h>
#include <stddef.h>
#include <float.h>
#include <limits.h>

typedef float          float32_t;
typedef double         float64_t;

struct uint128_t
{
	uint64_t word[2];
};
typedef struct uint128_t uint128_t;

struct uint256_t
{
	uint64_t word[4];
};
typedef struct uint256_t uint256_t;

#define FLOAT32_C(x)   (x##f)
#define FLOAT64_C(x)   (x)

#define FOUNDATION_SIZE_REAL 32

#if FOUNDATION_SIZE_REAL == 64
typedef   float64_t         real;
#  define REAL_C(x)         FLOAT64_C(x)
#else
typedef   float32_t         real;
#  define REAL_C(x)         FLOAT32_C(x)
#endif

//Pointer size
#if FOUNDATION_ARCH_ARM_64 || FOUNDATION_ARCH_X86_64 || FOUNDATION_ARCH_PPC_64 || FOUNDATION_ARCH_IA64 || FOUNDATION_ARCH_MIPS_64
#  define FOUNDATION_SIZE_POINTER 8
#else
#  define FOUNDATION_SIZE_POINTER 4
#endif

//wchar_t size
#if FOUNDATION_PLATFORM_LINUX_RASPBERRYPI
#  define FOUNDATION_SIZE_WCHAR 4
#else
#  if WCHAR_MAX > 0xffff
#    define FOUNDATION_SIZE_WCHAR 4
#  else
#    define FOUNDATION_SIZE_WCHAR 2
#  endif
#endif

//Atomic types
FOUNDATION_ALIGNED_STRUCT( atomic32_t, 4 )
{
	int32_t nonatomic;
};
typedef struct atomic32_t atomic32_t;

FOUNDATION_ALIGNED_STRUCT( atomic64_t, 8 )
{
	int64_t nonatomic;
};
typedef struct atomic64_t atomic64_t;

FOUNDATION_ALIGNED_STRUCT( atomicptr_t, FOUNDATION_SIZE_POINTER )
{
	void* nonatomic;
};
typedef struct atomicptr_t atomicptr_t;


//Pointer arithmetic
#define pointer_offset( ptr, ofs ) (void*)((char*)(ptr) + (ptrdiff_t)(ofs))
#define pointer_offset_const( ptr, ofs ) (const void*)((const char*)(ptr) + (ptrdiff_t)(ofs))
#define pointer_diff( first, second ) (ptrdiff_t)((const char*)(first) - (const char*)(second))


#include <string.h>


// Base limits
#define FOUNDATION_MAX_PATHLEN    512

#define FOUNDATION_UNUSED(x) (void)(x)

// Wrappers for platforms that not yet support thread-local storage declarations
#if FOUNDATION_PLATFORM_APPLE || FOUNDATION_PLATFORM_ANDROID

// Forward declarations of various system APIs
#if FOUNDATION_PLATFORM_APPLE
typedef __darwin_pthread_key_t _pthread_key_t;
#else
typedef int _pthread_key_t;
#  endif
FOUNDATION_EXTERN int pthread_key_create( _pthread_key_t*, void (*)(void*) );
FOUNDATION_EXTERN int pthread_setspecific( _pthread_key_t, const void* );
FOUNDATION_EXTERN void* pthread_getspecific( _pthread_key_t );

FOUNDATION_API void* _allocate_thread_local_block( unsigned int size );

#define FOUNDATION_DECLARE_THREAD_LOCAL( type, name, init ) \
static _pthread_key_t _##name##_key = 0; \
static FOUNDATION_FORCEINLINE _pthread_key_t get_##name##_key( void ) { if( !_##name##_key ) { pthread_key_create( &_##name##_key, 0 ); pthread_setspecific( _##name##_key, (init) ); } return _##name##_key; } \
static FOUNDATION_FORCEINLINE type get_thread_##name( void ) { return (type)((uintptr_t)pthread_getspecific( get_##name##_key() )); } \
static FOUNDATION_FORCEINLINE void set_thread_##name( type val ) { pthread_setspecific( get_##name##_key(), (const void*)(uintptr_t)val ); }

#define FOUNDATION_DECLARE_THREAD_LOCAL_ARRAY( type, name, arrsize ) \
static _pthread_key_t _##name##_key = 0; \
static FOUNDATION_FORCEINLINE _pthread_key_t get_##name##_key( void ) { if( !_##name##_key ) pthread_key_create( &_##name##_key, 0 ); return _##name##_key; } \
static FOUNDATION_FORCEINLINE type* get_thread_##name( void ) { _pthread_key_t key = get_##name##_key(); type* arr = (type*)pthread_getspecific( key ); if( !arr ) { arr = _allocate_thread_local_block( sizeof( type ) * arrsize ); pthread_setspecific( key, arr ); } return arr; }

#elif FOUNDATION_PLATFORM_WINDOWS && FOUNDATION_COMPILER_CLANG

__declspec(dllimport) unsigned long STDCALL TlsAlloc();
__declspec(dllimport) void* STDCALL TlsGetValue( unsigned long );
__declspec(dllimport) int STDCALL TlsSetValue( unsigned long, void* );

FOUNDATION_API void* _allocate_thread_local_block( unsigned int size );

#define FOUNDATION_DECLARE_THREAD_LOCAL( type, name, init ) \
static unsigned long _##name##_key = 0; \
static FOUNDATION_FORCEINLINE unsigned long get_##name##_key( void ) { if( !_##name##_key ) { _##name##_key = TlsAlloc(); TlsSetValue( _##name##_key, init ); } return _##name##_key; } \
static FOUNDATION_FORCEINLINE type get_thread_##name( void ) { return (type)((uintptr_t)TlsGetValue( get_##name##_key() )); } \
static FOUNDATION_FORCEINLINE void set_thread_##name( type val ) { TlsSetValue( get_##name##_key(), (void*)((uintptr_t)val) ); }

#define FOUNDATION_DECLARE_THREAD_LOCAL_ARRAY( type, name, arrsize ) \
static unsigned long _##name##_key = 0; \
static FOUNDATION_FORCEINLINE unsigned long get_##name##_key( void ) { if( !_##name##_key ) _##name##_key = TlsAlloc(); return _##name##_key; } \
static FOUNDATION_FORCEINLINE type* get_thread_##name( void ) { unsigned long key = get_##name##_key(); type* arr = (type*)TlsGetValue( key ); if( !arr ) { arr = _allocate_thread_local_block( sizeof( type ) * arrsize ); TlsSetValue( key, arr ); } return arr; }

#else

#define FOUNDATION_DECLARE_THREAD_LOCAL( type, name, init ) \
static FOUNDATION_THREADLOCAL type _thread_##name = init; \
static FOUNDATION_FORCEINLINE void set_thread_##name( type val ) { _thread_##name = val; } \
static FOUNDATION_FORCEINLINE type get_thread_##name( void ) { return _thread_##name; }

#define FOUNDATION_DECLARE_THREAD_LOCAL_ARRAY( type, name, arrsize ) \
static FOUNDATION_THREADLOCAL type _thread_##name [arrsize] = {0}; \
static FOUNDATION_FORCEINLINE type* get_thread_##name( void ) { return _thread_##name; }

#endif


//Utility functions for large integer types
static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL uint128_t uint128_make( const uint64_t low, const uint64_t high ) { uint128_t u = { { low, high } }; return u; }
static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL uint128_t uint128_null( void ) { return uint128_make( 0, 0 ); }
static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL bool      uint128_equal( const uint128_t u0, const uint128_t u1 ) { return u0.word[0] == u1.word[0] && u0.word[1] == u1.word[1]; }
static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL bool      uint128_is_null( const uint128_t u0 ) { return !u0.word[0] && !u0.word[1]; }

static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL uint256_t uint256_make( const uint64_t w0, const uint64_t w1, const uint64_t w2, const uint64_t w3 ) { uint256_t u = { { w0, w1, w2, w3 } }; return u; }
static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL uint256_t uint256_null( void ) { return uint256_make( 0, 0, 0, 0 ); }
static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL bool      uint256_equal( const uint256_t u0, const uint256_t u1 ) { return u0.word[0] == u1.word[0] && u0.word[1] == u1.word[1] && u0.word[2] == u1.word[2] && u0.word[3] == u1.word[3]; }
static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL bool      uint256_is_null( const uint256_t u0 ) { return !u0.word[0] && !u0.word[1] && !u0.word[2] && !u0.word[3]; }


//Format specifiers for 64bit and pointers
#if defined( _MSC_VER )
#  define PRId64       "I64d"
#  define PRIi64       "I64i"
#  define PRIdPTR      "Id"
#  define PRIiPTR      "Ii"
#  define PRIo64       "I64o"
#  define PRIu64       "I64u"
#  define PRIx64       "I64x"
#  define PRIX64       "I64X"
#  define PRIoPTR      "Io"
#  define PRIuPTR      "Iu"
#  define PRIxPTR      "Ix"
#  define PRIXPTR      "IX"
#else
#  ifndef __STDC_FORMAT_MACROS
#    define __STDC_FORMAT_MACROS
#  endif
#  include <inttypes.h>
#endif

#if FOUNDATION_SIZE_REAL == 64
#  define PRIREAL      "llf"
#else
#  define PRIREAL      "f"
#endif

#if FOUNDATION_PLATFORM_WINDOWS
#  if FOUNDATION_SIZE_POINTER == 8
#    define PRIfixPTR  "016I64X"
#  else
#    define PRIfixPTR  "08IX"
#  endif
#else
#  if FOUNDATION_SIZE_POINTER == 8
#    define PRIfixPTR  "016llX"
#  else
#    define PRIfixPTR  "08X"
#  endif
#endif

#include <foundation/build.h>
