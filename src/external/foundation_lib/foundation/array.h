/* array.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#include <foundation/platform.h>
#include <foundation/types.h>


#define array_deallocate( array )                           /*lint -e{522}*/ ( _array_verify( array ) ? memory_deallocate( _array_raw( array ) ), ( (array) = 0 ) : 0 )
#define array_capacity( array )                             ( _array_verify( array ) ? _array_rawcapacity_const( array ) : 0 )
#define array_reserve( array, capacity )                    ( (void)_array_maybegrowfixed( array, (int)(capacity) - array_capacity( array ) ) )
#define array_size( array )                                 ( _array_verify( array ) ? _array_rawsize_const( array ) : 0 )
#define array_grow( array, num )                            ( (void)_array_resize( array, array_size( array ) + num ) )
#define array_resize( array, num )                          ( (void)_array_resize( array, num ) )
#define array_clear( array )                                ( _array_verify( array ) ? ( _array_rawsize( array ) = 0 ) : 0 )
#define array_copy( dst, src )                              ( _array_verify( src ) && ( _array_elementsize( src ) == _array_elementsize( dst ) ) ? ( _array_maybegrowfixed( (dst), ( _array_rawsize_const( src ) - ( _array_verify( dst ) ? ( _array_rawsize( dst ) ) : 0 ) ) ) ), memcpy( (dst), (src), ( _array_rawsize_const( src ) ) * _array_elementsize( src ) ), ( _array_rawsize( dst ) = _array_rawsize_const( src ) ) : array_clear( dst ) )
#define array_push( array, element )                        ( (void)_array_maybegrow( array, 1 ), (array)[ _array_rawsize( array )++ ] = (element) )
#define array_push_memcpy( array, elementptr )              /*lint -e{506}*/ ( (void)_array_maybegrow( array, 1 ), memcpy( (array) + _array_rawsize( array )++, (elementptr), sizeof( *(array) ) ) )
#define array_insert( array, pos, element )                 ( (void)_array_maybegrow( array, 1 ), memmove( (array) + (pos) + 1, (array) + (pos), _array_elementsize( array ) * ( _array_rawsize( array )++ - (pos) ) ), (array)[(pos)] = (element) )
#define array_insert_memcpy( array, pos, elementptr )       ( (void)_array_maybegrow( array, 1 ), memmove( (array) + (pos) + 1, (array) + (pos), _array_elementsize( array ) * ( _array_rawsize( array )++ - (pos) ) ), memcpy( (array) + (pos), (elementptr), sizeof( *(array) ) ) )
#define array_insert_safe( array, pos, element )            do { int _clamped_pos = math_clamp( (pos), 0, array_size( array ) ); array_insert( array, _clamped_pos, element ); } while(0)
#define array_insert_memcpy_safe( array, pos, elementptr )  do { int _clamped_pos = math_clamp( (pos), 0, array_size( array ) ); array_insert_memcpy( array, _clamped_pos, elementptr ); } while(0)
#define array_pop( array )                                  ( _array_verify( array ) ? --_array_rawsize( array ) : 0 )
#define array_pop_safe( array )                             ( _array_verify( array ) && ( _array_rawsize( array ) > 0 ) ? --_array_rawsize( array ) : 0 )
#define array_erase( array, pos )                           ( _array_verify( array ) ? *((array) + (pos)) = *((array) + ( _array_rawsize( array ) - 1 )), --_array_rawsize( array ) : 0 )
#define array_erase_memcpy( array, pos )                    ( _array_verify( array ) ? memcpy( (array) + (pos), (array) + ( _array_rawsize( array ) - 1 ), _array_elementsize( array ) ), --_array_rawsize( array ) : 0 )
#define array_erase_safe( array, pos )                      ( _array_verify( array ) && _array_verify_index( array, pos ) ? array_erase( array, pos ) : 0 )
#define array_erase_memcpy_safe( array, pos )               ( _array_verify( array ) && _array_verify_index( array, pos ) ? array_erase_memcpy( array, pos ) : 0 )
#define array_erase_ordered( array, pos )                   ( _array_verify( array ) ? memmove( (array) + (pos), (array) + (pos) + 1, ( _array_rawsize( array ) - (pos) - 1 ) * _array_elementsize( array ) ), --_array_rawsize( array ) : 0 )
#define array_erase_ordered_safe( array, pos )              ( _array_verify( array ) && _array_verify_index( array, pos ) ? array_erase_ordered( array, pos ) : 0 )
#define array_erase_ordered_range( array, pos, num )        ( _array_verify( array ) && ( (int32_t)(num) > 0 ) ? memmove( (array) + (pos), (array) + (pos) + (num), ( _array_rawsize( array ) - (pos) - (num) ) * _array_elementsize( array ) ), ( _array_rawsize( array ) -= (num) ) : 0 )
#define array_erase_ordered_range_safe( array, pos, num )   do { int _clamped_start = math_clamp( (pos), 0, array_size( array ) ); int _clamped_end = math_clamp( ( (pos) + (num) ), 0, array_size( array ) ); if( _clamped_end > _clamped_start ) array_erase_ordered_range( array, _clamped_start, _clamped_end - _clamped_start ); } while(0)


// **** Internal implementation details below, not for direct use ****

// Header size set to 16 bytes in order to align main array memory
#define _array_header_size           4UL
#if BUILD_DEBUG
#  define _array_verify(a)           ( _array_verifyfn((const void* const*)&(a)) )
#else
#  define _array_verify(a)           ( a )
#endif
#define _array_raw(a)                ( (int32_t*)(a) - _array_header_size )
#define _array_rawcapacity(a)        _array_raw(a)[0]
#define _array_rawsize(a)            _array_raw(a)[1]
#define _array_rawelementsize(a)     _array_raw(a)[3]
#define _array_raw_const(a)          ( (const int32_t*)(a)-_array_header_size )
#define _array_rawcapacity_const(a)  _array_raw_const(a)[0]
#define _array_rawsize_const(a)      _array_raw_const(a)[1]
#define _array_rawelementsize_const(a) _array_raw_const(a)[3]

#define _array_elementsize(a)        ( (int)(pointer_diff( &(a)[1], &(a)[0] )) )
#define _array_needgrow(a,n)         ( ((n)>0) && ( _array_verify(a)==0 || (_array_rawsize_const(a)+(n)) > _array_rawcapacity_const(a) ) )
#define _array_maybegrow(a,n)        ( _array_needgrow(a,(n)) ? _array_grow(a,n,2) : (a) )
#define _array_maybegrowfixed(a,n)   ( _array_needgrow(a,(n)) ? _array_grow(a,n,1) : (a) )
#define _array_grow(a,n,f)           ( _array_growfn((void**)&(a),(n),(f),_array_elementsize(a)) )
#define _array_resize(a,n)           ( _array_resizefn((void**)&(a),(n),_array_elementsize(a)) )

#define _array_verify_index(a,n)     ( ( (int32_t)(n) < _array_rawsize(a) ) && ( (int32_t)(n) >= 0 ) )

FOUNDATION_API void*                 _array_growfn( void** arr, int increment, int factor, int itemsize );
FOUNDATION_API void*                 _array_resizefn( void** arr, int elements, int itemsize );
FOUNDATION_API const void*           _array_verifyfn( const void* const* arr );

