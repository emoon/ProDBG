#pragma once

#define sizeof_array(t) (sizeof(t)/sizeof(t[0]))

// \todo: Add Nucleus as a submodule (need to clear privacy first though)

#define NcOn 3 -
#define NcOff 2 -
#define NcFeature(feature) (feature 1 == 2)


#define NcPlatformApple NcOff
#define NcPlatformWin32 NcOff
#define NcPlatformWin64 NcOff
#define NcPlatformWindows NcOff

// Apple
#if defined(__APPLE__) && __APPLE__
    #undef NcPlatformApple
    #define NcPlatformApple NcOn

// Windows
#elif defined(_WIN32) || defined(_WIN64)
    #undef NcPlatformWindows
    #define NcPlatformWindows NcOn
    #ifdef _WIN64
        #undef NcPlatformWin64
        #define NcPlatformWin64 NcOn
    #else
        #undef NcPlatformWin32
        #define NcPlatformWin32 NcOn
    #endif

// Unknown
#else
    #error

#endif


#define NcProcessorX64 NcOff
#define NcProcessorX86 NcOff

// x86-64
// __x86_64__ is defined by GCC/Clang
// __amd64__ is defined by GCC/Clang
// _M_X64 is defined by MSVC
#if defined(__x86_64__) || defined(__amd64__) || defined(_M_X64)
    #undef NcProcessorX64
    #define NcProcessorX64 NcOn

// x86
// __i386__ is defined by GCC/Clang
// _M_IX86 is defined by MSVC
#elif defined(__i386__) || defined(_M_IX86)
    #undef NcProcessorX86
    #define NcProcessorX86 NcOn

// Unknown
#else
    #error

#endif


// Pointer Size
#ifndef NcPlatformPointerSize
    #if NcFeature(NcProcessorX64)
        #define NcPlatformPointerSize 8
    #else
        //! \brief The size of a pointer on the target platform.
        #define NcPlatformPointerSize 4
    #endif
#endif


// Endianness
#define NcBigEndian NcOff


#define NcCompilerClang NcOff
#define NcCompilerGcc NcOff
#define NcCompilerMsvc NcOff

// Clang
#if defined(__clang__)
    #undef NcCompilerClang
    #define NcCompilerClang NcOn

// GCC
#elif defined(__GNUC__)
    #undef NcCompilerGcc
    #define NcCompilerGcc NcOn

    //! \cxx11-todo Temporary until GCC supports this.
    #define alignas(alignmentInBytes) __attribute__((aligned(alignmentInBytes)))

// Microsoft Visual C++
#elif defined(_MSC_VER)
    #undef NcCompilerMsvc
    #define NcCompilerMsvc NcOn

    //! \cxx11-todo Temporary until MSVC supports these.
    #ifndef _ALLOW_KEYWORD_MACROS
    #define _ALLOW_KEYWORD_MACROS
    #endif
    #define alignas(alignmentInBytes) __declspec(align(alignmentInBytes))
    #define alignof __alignof
    #define final sealed

// Unknown
#else
    #error

#endif





#if NcFeature(NcCompilerClang)
    //#define offsetof __builtin_offsetof
#else
    //#include <stddef.h>
    //! \brief Expands to the offset of \a field within \a type.
    //#define offsetof(type, field) ::nc::uintptr(&reinterpret_cast<char const volatile&>(static_cast<type*>(nullptr)->field))
#endif


namespace nc
{
    //! \brief Helper for NcArrayCount. \internal
    template<class T, long size> char (&arrayCountHelper(T (&arr)[size]))[size];
}

//! \brief Expands to the number of elements in \a fixedSizeArray.
#define NcArrayCount(fixedSizeArray) sizeof(::nc::arrayCountHelper(fixedSizeArray))


#if NcFeature(NcCompilerMsvc)
    //! \brief Require a type to be packed. This ignores alignment requirements of its members.
    #define NcPacked()
#elif NcFeature(NcCompilerClang) || NcFeature(NcCompilerGcc)
    #define NcPacked() __attribute__((packed))
#else
    #error
#endif

#if NcFeature(NcCompilerMsvc)
    //! \brief Require types declared between this and the next NcRestorePacked to be packed.
    #define NcRequirePacked() __pragma(pack(push, 1))
    //! \brief Restores the previous type packing setting.
    #define NcRestorePacked() __pragma(pack(pop))
#else
    #define NcRequirePacked()
    #define NcRestorePacked()
#endif


#if NcFeature(NcPlatformWindows)
    //! \brief Mark the symbol as exported from a shared library.
    #define NcExport __declspec(dllexport)
    //! \brief Mark the symbol as imported from a shared library.
    #define NcImport __declspec(dllimport)
#else
    #define NcExport
    #define NcImport
#endif


#if NcFeature(NcCompilerMsvc)
    //! \brief Forces the compiler to inline the target function.
    #define NcForceInline __forceinline
#elif NcFeature(NcCompilerClang) || NcFeature(NcCompilerGcc)
    #define NcForceInline __attribute__((always_inline)) inline
#else
    #error
#endif

#if NcFeature(NcCompilerMsvc)
    //! \brief Forbids the compiler from inlining the target function.
    #define NcNeverInline __declspec(noinline)
#elif NcFeature(NcCompilerClang) || NcFeature(NcCompilerGcc)
    #define NcNeverInline __attribute__((noinline))
#else
    #error
#endif


#if NcFeature(NcCompilerMsvc)
    //! \brief Declares a thread local variable.
    #define NcThreadLocal __declspec(thread)
#elif NcFeature(NcCompilerClang) || NcFeature(NcCompilerGcc)
    #define NcThreadLocal __thread
#else
    #error
#endif


#ifndef NcDisableWarningMsvc
    #if NcFeature(NcCompilerMsvc)
        //! \brief Disable warning \a number until a matching NcRestoreWarningMsvc.
        #define NcDisableWarningMsvc(number) \
                __pragma(warning(push)) \
                __pragma(warning(disable : number))
    #else
        #define NcDisableWarningMsvc(number)
    #endif
#endif

#ifndef NcRestoreWarningMsvc
    #if NcFeature(NcCompilerMsvc)
        //! \brief Restore the warning disabled by the most recent NcDisableWarningMsvc.
        #define NcRestoreWarningMsvc() \
                __pragma(warning(pop))
    #else
        #define NcRestoreWarningMsvc()
    #endif
#endif


//! \brief The default memory alignment to use for the target platform and configuration.
#define NcDefaultAlignment 16

static_assert(!(NcDefaultAlignment & (NcDefaultAlignment - 1)),
              "NcDefaultAlignment must be a power of two");


//! \brief Converts \a expr into a string literal with macro expansion.
#define NcString(expr) NcStringNoEx(expr)

//! \brief Converts \a expr into a string literal without macro expansion.
#define NcStringNoEx(expr) #expr

//! \brief Combines \a x and \a y into a single token with macro expansion.
#define NcPaste(x, y) NcPasteNoEx(x, y)

//! \brief Combines \a x and \a y into a single token without macro expansion.
#define NcPasteNoEx(x, y) x ## y


//! \brief Disallow copy construction.
//!
//! Use at the top of a class declaration or in a private region of a struct.
#define NcDisableCopy(Type) \
        Type(Type const&) = delete

//! \brief Disallow copy assignment.
//!
//! Use at the top of a class declaration or in a private region of a struct.
#define NcDisableAssign(Type) \
        Type& operator =(Type const&) = delete

//! \brief Disallow copy construction and copy assignment.
//!
//! Use at the top of a class declaration or in a private region of a struct.
#define NcDisableCopyAssign(Type) \
        NcDisableCopy(Type); \
        NcDisableAssign(Type)


