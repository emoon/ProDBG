/*
 *     xDMS  v1.3  -  Portable DMS archive unpacker  -  Public Domain
 *     Written by     Andre Rodrigues de la Rocha  <adlroc@usa.net>
 *
 *     Main types of variables used in xDMS, some implementation
 *     dependant features and other global stuff
 */
#ifndef __MSDOS
#include <stdint.h>
#endif

#ifndef UCHAR
  #ifdef __MSDOS
  #define UCHAR unsigned char
 #else
  #define UCHAR uint8_t
 #endif
#endif

#ifndef USHORT
 #ifdef __MSDOS
  #define USHORT unsigned int
 #else
  #define USHORT uint16_t
 #endif
#endif


#ifndef SHORT
 #ifdef __MSDOS
  #define SHORT short
 #else
  #define SHORT int16_t
 #endif
#endif

#ifndef ULONG
  #ifdef __MSDOS
  #define ULONG unsigned long
 #else
  #define ULONG uint32_t
 #endif
#endif


#ifndef INLINE
	#ifdef __cplusplus
		#define INLINE inline
	#else
		#ifdef __GNUC__
			#define INLINE inline static
		#else
			#ifdef __SASC
				#define INLINE __inline
			#else
				#define INLINE static
			#endif
		#endif
	#endif
#endif


#ifndef UNDER_DOS
	#ifdef __MSDOS__
		#define UNDER_DOS
	#else
		#ifdef __MSDOS
			#define UNDER_DOS
		#else
			#ifdef _OS2
				#define UNDER_DOS
			#else
				#ifdef _QC
					#define UNDER_DOS
				#endif
			#endif
		#endif
	#endif
#endif


#ifndef DIR_CHAR
	#ifdef UNDER_DOS
		/* running under MSDOS or DOS-like OS */
		#define DIR_CHAR '\\'
	#else
		#define DIR_CHAR '/'
	#endif
#endif


#define DIR_SEPARATORS ":\\/"


extern UCHAR *text;
