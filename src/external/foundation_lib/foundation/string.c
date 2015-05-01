/* string.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#include <foundation/foundation.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if FOUNDATION_PLATFORM_WINDOWS
FOUNDATION_EXTERN errno_t _ctime64_s( char*, size_t, const __time64_t* );;
#elif FOUNDATION_PLATFORM_APPLE
FOUNDATION_EXTERN char* ctime_r( const time_t*, char* );
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
#include <time.h>
#endif


char* string_allocate( unsigned int length )
{
	char* str = memory_allocate( HASH_STRING, length + 1, 0, MEMORY_PERSISTENT );
	str[0] = 0;
	return str;
}


void string_deallocate( char* str )
{
	memory_deallocate( str );
}


char* string_clone( const char* str )
{
	if( str )
	{
		unsigned int len = string_length( str ) + 1;
		char* clone = memory_allocate( HASH_STRING, len, 0, MEMORY_PERSISTENT );
		memcpy( clone, str, len );
		return clone;
	}
	return 0;
}


char* string_format( const char* format, ... )
{
	int n;
	unsigned int capacity;
	char* buffer;
	va_list list;

	if( !format )
		return 0;

	capacity = string_length( format ) + 32;
	buffer = memory_allocate( HASH_STRING, capacity + 1, 0, MEMORY_PERSISTENT );

	while( 1 )
	{
		va_start( list, format );
		n = vsnprintf( buffer, capacity, format, list );
		va_end( list );

		if( ( n > -1 ) && ( n < (int)capacity ) )
			break;

		if( n > -1 )
			capacity = n + 1;
		else
			capacity *= 2;

		buffer = memory_reallocate( buffer, capacity + 1, 0, 0 );
	}

	return buffer;
}


char* string_format_buffer( char* buffer, unsigned int maxlen, const char* format, ... )
{
	va_list list;

	if( !buffer )
		return 0;
	if( !format )
	{
		buffer[0] = 0;
		return buffer;
	}

	va_start( list, format );
	vsnprintf( buffer, maxlen, format, list );
	va_end( list );

	return buffer;
}


char* string_vformat( const char* format, va_list list )
{
	int n;
	unsigned int capacity;
	char* buffer;
	va_list copy_list;

	if( !format )
		return 0;

	capacity = string_length( format ) + 32;
	buffer = memory_allocate( HASH_STRING, capacity + 1, 0, MEMORY_PERSISTENT );

	while( 1 )
	{
		va_copy( copy_list, list );
		n = vsnprintf( buffer, capacity, format, copy_list );
		va_end( copy_list );

		if( ( n > -1 ) && ( n < (int)capacity ) )
			break;

		if( n > -1 )
			capacity = n + 1;
		else
			capacity *= 2;

		buffer = memory_reallocate( buffer, capacity + 1, 0, 0 );
	}

	return buffer;
}


char* string_vformat_buffer( char* buffer, unsigned int maxlen, const char* format, va_list list )
{
	va_list copy_list;

	if( !buffer )
		return 0;
	if( !format )
	{
		buffer[0] = 0;
		return buffer;
	}

	va_copy( copy_list, list );
	vsnprintf( buffer, maxlen, format, copy_list );
	va_end( copy_list );

	return buffer;
}


unsigned int string_length( const char* str )
{
	return str ? (unsigned int)strlen( str ) : 0;
}


hash_t string_hash( const char* str )
{
	return str ? hash( str, string_length( str ) ) : HASH_EMPTY_STRING;
}


char* string_resize( char* str, unsigned int length, char c )
{
	unsigned int curlen = string_length( str );

	if( curlen < length )
	{
		str = memory_reallocate( str, length + 1, 0, curlen );
		memset( str + curlen, c, length - curlen );
	}
	if( str )
		str[length] = 0;
	return str;
}


void string_copy( char* dst, const char* src, unsigned int limit )
{
	unsigned int length = string_length( src );
	if( length >= limit )
		length = limit - 1;
	if( dst )
	{
		memcpy( dst, src, length );
		dst[length] = 0;
	}
}


char* string_strip( char* str, const char* delimiters )
{
	unsigned int start, end, length, newlength;

	if( !str )
		return 0;

	length = string_length( str );
	start = string_find_first_not_of( str, delimiters, 0 );
	end   = string_rfind_first_not_of( str, delimiters, length - 1 );

	if( start != STRING_NPOS )
	{
		if( end == STRING_NPOS )
			end = length - 1;
		newlength = end - start + 1;
		if( start != 0 )
			memmove( str, str + start, newlength );
		str[newlength] = 0;
	}
	else
	{
		str[0] = 0;
	}

	return str;
}


char* string_strip_substr( char* str, const char* delimiters, unsigned int length )
{
	unsigned int start, end, newlength;

	if( !str || !length )
		return 0;

	start = string_find_first_not_of( str, delimiters, 0 );
	end   = string_rfind_first_not_of( str, delimiters, length - 1 );

	if( ( start != STRING_NPOS ) && ( start < length ) )
	{
		if( end == STRING_NPOS )
			end = length - 1;
		newlength = end - start + 1;
		if( start != 0 )
			memmove( str, str + start, newlength );
		str[newlength] = 0;
	}
	else
	{
		str[0] = 0;
	}

	return str;
}


char* string_replace( char* str, const char* key, const char* newkey, bool repeat )
{
	unsigned int pos, lastpos, keylen, newkeylen, slen;
	int lendiff, replaced;

	slen = (unsigned int)string_length( str );
	keylen = (unsigned int)string_length( key );
	if( !slen || !keylen || string_equal( key, newkey ) )
		return str;

	lastpos = STRING_NPOS;
	newkeylen = newkey ? (unsigned int)string_length( newkey ) : 0;
	lendiff = (int)newkeylen - (int)keylen;
	pos = 0;
	replaced = 0;

	while( ( pos = string_find_string( str, key, pos ) ) != STRING_NPOS )
	{
		if( repeat && ( lastpos != STRING_NPOS ) && ( (int)pos <= ( (int)lastpos + lendiff ) ) )
		{
			//Avoid infinite loop (same position, string did not reduce)
			pos = lastpos + newkeylen;
			continue;
		}

		if( lendiff <= 0 )
		{
			//String is reducing or keeping length, just overwrite
			memcpy( str + pos, newkey, newkeylen );
			memmove( str + pos + newkeylen, str + pos + keylen, slen - pos - keylen + 1 );
			slen += lendiff; //Zero or negative, so reducing by adding
		}
		else
		{
			str = memory_reallocate( str, slen + lendiff + 1, 0, slen + 1 );
			memmove( str + pos + newkeylen, str + pos + keylen, slen - pos - keylen + 1 );
			memcpy( str + pos, newkey, newkeylen );

			slen += lendiff;
		}

		++replaced;

		lastpos = pos;
		if( !repeat )
			pos += newkeylen;
	}

	if( replaced )
		str[ slen ] = 0;

	return str;
}


char* string_append( char* str, const char* suffix )
{
	unsigned int slen = str ? string_length( str ) : 0;
	unsigned int suffixlen = suffix ? string_length( suffix ) : 0;
	unsigned int totallen = slen + suffixlen;
	if( !suffixlen )
		return str;

	str = str ? memory_reallocate( str, totallen + 1, 0, slen ) : memory_allocate( HASH_STRING, totallen + 1, 0, MEMORY_PERSISTENT );
	memcpy( str + slen, suffix, suffixlen + 1 ); //Include terminating zero

	return str;
}


char* string_prepend( char* str, const char* prefix )
{
	unsigned int slen = str ? string_length( str ) : 0;
	unsigned int prefixlen = prefix ? string_length( prefix ) : 0;
	unsigned int totallen = slen + prefixlen;
	if( !prefixlen )
		return str;

	str = str ? memory_reallocate( str, totallen + 1, 0, slen + 1 ) : memory_allocate( HASH_STRING, totallen + 1, 0, MEMORY_PERSISTENT );
	if( slen )
		memmove( str + prefixlen, str, slen + 1 ); //Include terminating zero
	memcpy( str, prefix, prefixlen );
	if( !slen )
		str[prefixlen] = 0;

	return str;
}


char* string_concat( const char* lhs, const char* rhs )
{
	unsigned int llen = string_length( lhs );
	unsigned int rlen = string_length( rhs );
	char* buf = memory_allocate( HASH_STRING, llen + rlen + 1, 0, MEMORY_PERSISTENT );
	if( llen )
		memcpy( buf, lhs, llen );
	if( rlen )
		memcpy( buf + llen, rhs, rlen );
	buf[ llen + rlen ] = 0;
	return buf;
}


void string_split( const char* str, const char* separators, char** left, char** right, bool allowempty )
{
	unsigned int start, delim;

	start = ( allowempty ? 0 : string_find_first_not_of( str, separators, 0 ) );
	if( start == STRING_NPOS )
	{
		if( left )
			*left = 0;
		if( right )
			*right = 0;
		return;
	}

	delim = string_find_first_of( str, separators, start );
	if( delim != STRING_NPOS )
	{
		if( left )
			*left = string_substr( str, start, delim - start );
		if( right )
		{
			delim = ( allowempty ? ( delim + 1 ) : string_find_first_not_of( str, separators, delim ) );
			if( delim != STRING_NPOS )
				*right = string_substr( str, delim, STRING_NPOS );
			else
				*right = 0;
		}
	}
	else
	{
		if( left )
			*left = string_substr( str, start, STRING_NPOS );
		if( right )
			*right = 0;
	}
}


char* string_substr( const char* str, unsigned int offset, unsigned int length )
{
	char* buffer;
	unsigned int newlen;
	unsigned int slen = string_length( str );
	if( !str || ( offset >= slen ) || !slen )
		return string_allocate( 0 );
	newlen = slen - offset;
	if( length < newlen )
		newlen = length;
	buffer = memory_allocate( HASH_STRING, newlen + 1, 0, MEMORY_PERSISTENT );
	memcpy( buffer, str + offset, newlen );
	buffer[newlen] = 0;
	return buffer;
}


unsigned int string_find( const char* str, char c, unsigned int offset )
{
	const char* found;
	FOUNDATION_ASSERT( ( offset == STRING_NPOS ) || ( offset <= strlen( str ) ) );
	if( offset == STRING_NPOS )
		return STRING_NPOS;
	found = strchr( str + offset, c );
	if( found )
		return (unsigned int)(uintptr_t)( found - str );
	return STRING_NPOS;
}


unsigned int string_find_string( const char* str, const char* key, unsigned int offset )
{
	const char* found;
	FOUNDATION_ASSERT( ( offset == STRING_NPOS ) || ( offset <= strlen( str ) ) );
	if( offset == STRING_NPOS )
		return STRING_NPOS;
	found = strstr( str + offset, key );
	if( found )
		return (unsigned int)(uintptr_t)( found - str );
	return STRING_NPOS;
}


unsigned int string_rfind( const char* str, char c, unsigned int offset )
{
	FOUNDATION_ASSERT( ( offset == STRING_NPOS ) || ( offset <= strlen( str ) ) );
	if( offset == STRING_NPOS )
		offset = string_length( str ) - 1; //Wrap-around caught by if clause below

	while( offset != STRING_NPOS ) //Wrap-around terminates
	{
		if( c == str[ offset ] )
			return offset;
		--offset;
	}

	return STRING_NPOS;
}


unsigned int string_rfind_string( const char* str, const char* key, unsigned int offset )
{
	unsigned int keylen, slen;

	FOUNDATION_ASSERT( ( offset == STRING_NPOS ) || ( offset <= strlen( str ) ) );

	slen = (unsigned int)strlen( str );
	keylen = (unsigned int)strlen( key );
	if( slen && ( keylen <= slen ) )
	{
		if( offset > slen )
			offset = slen;
		if( !keylen )
			return ( offset <= slen ? offset : slen );

		do
		{
			if( !strncmp( str + offset, key, keylen ) )
				return offset;
			--offset;
		} while( offset < slen ); //Breaks out when wrap around
	}
	return STRING_NPOS;
}


unsigned int string_find_first_of( const char* str, const char* tokens, unsigned int offset )
{
	const char* found;
	FOUNDATION_ASSERT( ( offset == STRING_NPOS ) || ( offset <= strlen( str ) ) );
	if( offset == STRING_NPOS )
		return STRING_NPOS;
	found = strpbrk( str + offset, tokens );
	if( found )
		return (unsigned int)(uintptr_t)( found - str );
	return STRING_NPOS;
}


unsigned int string_find_last_of( const char* str, const char* tokens, unsigned int offset )
{
	FOUNDATION_ASSERT( ( offset == STRING_NPOS ) || ( offset <= strlen( str ) ) );
	if( offset == STRING_NPOS )
		offset = string_length( str ) - 1; //Wrap-around caught by if clause below

	if( offset != STRING_NPOS ) do
	{
		if( strchr( tokens, str[ offset ] ) && str[ offset ] )
			return offset;
		--offset;
	} while( offset != STRING_NPOS ); //Wrap-around terminates

	return STRING_NPOS;
}


unsigned int string_find_first_not_of( const char* str, const char* tokens, unsigned int offset )
{
	FOUNDATION_ASSERT( ( offset == STRING_NPOS ) || ( offset <= strlen( str ) ) );
	if( offset == STRING_NPOS )
		return STRING_NPOS;
	while( str[ offset ] )
	{
		if( !strchr( tokens, str[ offset ] ) )
			return offset;
		++offset;
	}
	return STRING_NPOS;
}


unsigned int string_find_last_not_of( const char* str, const char* tokens, unsigned int offset )
{
	FOUNDATION_ASSERT( ( offset == STRING_NPOS ) || ( offset <= strlen( str ) ) );
	if( offset == STRING_NPOS )
		offset = string_length( str ) - 1; //Wrap-around caught by if clause below

	if( offset != STRING_NPOS ) do
	{
		if( !strchr( tokens, str[ offset ] ) )
			return offset;
		--offset;
	} while( offset != STRING_NPOS ); //Wrap-around terminates

	return STRING_NPOS;
}


unsigned int string_rfind_first_of( const char* str, const char* tokens, unsigned int offset )
{
	FOUNDATION_ASSERT( ( offset == STRING_NPOS ) || ( offset <= strlen( str ) ) );
	if( offset == STRING_NPOS )
		offset = string_length( str ) - 1; //Wrap-around caught by if clause below

	if( offset != STRING_NPOS ) do
	{
		if( strchr( tokens, str[offset] ) )
			return offset;
		--offset;
	} while( offset != STRING_NPOS ); //Wrap-around terminates

	return STRING_NPOS;
}


unsigned int string_rfind_first_not_of( const char* str, const char* tokens, unsigned int offset )
{
	FOUNDATION_ASSERT( ( offset == STRING_NPOS ) || ( offset <= strlen( str ) ) );
	if( offset == STRING_NPOS )
		offset = string_length( str ) - 1; //Wrap-around caught by if clause below

	if( offset != STRING_NPOS ) do
	{
		if( !strchr( tokens, str[offset] ) )
			return offset;
		--offset;
	} while( offset != STRING_NPOS ); //Wrap-around terminates

	return STRING_NPOS;
}


bool string_ends_with( const char* str, const char* suffix )
{
	unsigned int len = string_length( str );
	unsigned int suffix_len = string_length( suffix );
	if( len < suffix_len )
		return false;
	return string_equal( str + ( len - suffix_len ), suffix );
}


bool string_equal( const char* rhs, const char* lhs )
{
	return ( rhs == lhs ) || ( rhs && lhs && ( strcmp( rhs, lhs ) == 0 ) ) || ( !rhs && lhs && lhs[0] == 0 ) || ( rhs && !lhs && rhs[0] == 0 );
}


bool string_equal_substr( const char* rhs, const char* lhs, unsigned int len )
{
	return ( rhs == lhs ) || ( rhs && lhs && ( strncmp( rhs, lhs, len ) == 0 ) ) || ( !rhs && lhs && ( !len || lhs[0] == 0 ) ) || ( rhs && !lhs && ( !len || rhs[0] == 0 ) );
}


bool string_match_pattern( const char* element, const char* pattern )
{
	if( ( *pattern == '*') && !pattern[1] )
		return true;
	else if( !*element && *pattern )
		return false;
	else if( !*element )
		return true;

	if( *pattern == '*' )
	{
		if( string_match_pattern( element, pattern + 1) )
			return true;
		return string_match_pattern( element + 1, pattern );
	}
	else if( *pattern == '?' )
		return string_match_pattern( element + 1, pattern + 1 );
	else if( *element == *pattern )
		return string_match_pattern( element + 1, pattern + 1 );

	return false;
}


char** string_explode( const char* str, const char* delimiters, bool allow_empty )
{
	char** array;
	unsigned int slen;
	unsigned int delimlen;
	unsigned int token;
	unsigned int end;

	slen = string_length( str );
	if( !slen )
		return 0;

	array = 0;
	delimlen = (unsigned int)strlen( delimiters );
	if( !delimlen )
	{
		array_push( array, string_clone( str ) );
		return array;
	}

	token = 0;
	end   = 0;

	while( end < slen )
	{
		if( !allow_empty )
			token = string_find_first_not_of( str, delimiters, end );
		end = string_find_first_of( str, delimiters, token );

		if( token != STRING_NPOS )
			array_push( array, string_substr( str, token, ( end != STRING_NPOS ) ? ( end - token ) : STRING_NPOS ) );
		if( allow_empty )
			token = end + 1;
	}

	return array;
}


char* string_merge( const char* const* array, unsigned int num, const char* delimiter )
{
	char* result;
	unsigned int i;

	if( !num )
		return string_allocate( 0 );

	result = string_clone( array[0] );
	for( i = 1; i < num; ++i )
	{
		result = string_append( result, delimiter );
		result = string_append( result, array[i] );
	}

	return result;
}


void string_array_deallocate_elements( char** array )
{
	int i, size = array_size( array );
	for( i = 0; i < size; ++i )
		string_deallocate( array[i] );
}


int string_array_find( const char* const* array, const char* needle, unsigned int haystack_size )
{
	unsigned int i;
	for( i = 0; i < haystack_size; ++i, ++array )
	{
		if( string_equal( *array, needle ) )
			return i;
	}
	return -1;
}



#define get_bit_mask( numbits ) ( ( 1 << (numbits) ) - 1 )


static unsigned int get_num_bytes_utf8( uint8_t lead )
{
	if(      ( lead & 0xFC ) == 0xF8 ) return 5;
	else if( ( lead & 0xF8 ) == 0xF0 ) return 4;
	else if( ( lead & 0xF0 ) == 0xE0 ) return 3;
	else if( ( lead & 0xE0 ) == 0xC0 ) return 2;
	else return 1;
}


static unsigned int get_num_bytes_as_utf8( uint32_t val )
{
	if(      val >= 0x04000000 ) return 6;
	else if( val >= 0x00200000 ) return 5;
	else if( val >= 0x00010000 ) return 4;
	else if( val >= 0x00000800 ) return 3;
	else if( val >= 0x00000080 ) return 2;
	return 1;
}


static unsigned int encode_utf8( char* str, uint32_t val )
{
	unsigned int num, j;

	if( val < 0x80 )
	{
		*str = (char)val;
		return 1;
	}

	//Get number of _extra_ bytes
	num = get_num_bytes_as_utf8( val ) - 1;

	*str++ = (char)( ( 0x80 | ( get_bit_mask( num ) << ( 7 - num ) ) ) | ( ( val >> ( 6 * num ) ) & get_bit_mask( 6 - num ) ) );
	for( j = 1; j <= num; ++j )
		*str++ = ( 0x80 | ( ( val >> ( 6 * ( num - j ) ) ) & 0x3F ) );

	return num + 1;
}


uint32_t string_glyph( const char* str, unsigned int offset, unsigned int* consumed )
{
	uint32_t glyph = 0;
	unsigned int num, j;
	const char* cur = str + offset;

	if( !( *cur & 0x80 ) )
	{
		glyph = *cur++;
		if( consumed )
			*consumed = 1;
	}
	else
	{
		//Convert through UTF-32
		num = get_num_bytes_utf8( *cur ) - 1; //Subtract one to get number of _extra_ bytes
		glyph = ( (uint32_t)(*cur) & get_bit_mask( 6 - num ) ) << ( 6 * num );
		++cur;
		for( j = 1; j <= num; ++j, ++cur )
			glyph |= ( (uint32_t)(*cur) & 0x3F ) << ( 6 * ( num - j ) );
		if( consumed )
			*consumed = num + 1;
	}
	return glyph;
}


unsigned int string_glyphs( const char* str )
{
	unsigned int num = 0;
	while( *str )
	{
		++num;
		str += get_num_bytes_utf8( *str );
	}
	return num;
}


wchar_t* wstring_allocate_from_string( const char* cstr, unsigned int length )
{
	wchar_t* buffer;
	wchar_t* dest;
	unsigned int maxlen, num_chars, num_bytes, i, j;
	uint32_t glyph;
	const char* cur;

	if( !cstr )
	{
		buffer = memory_allocate( HASH_STRING, sizeof( wchar_t ), 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );
		return buffer;
	}

	maxlen = (unsigned int)strlen( cstr );
	if( !length )
		length = maxlen;
	else
		length = ( length < maxlen ) ? length : maxlen;

	//Count number of wchar_t needed to represent string
	num_chars = 0;
	cur = cstr;
	for( i = 0; i < length; )
	{
		num_bytes = get_num_bytes_utf8( *cur );
		if( sizeof( wchar_t ) == 2 )
		{
			if( num_bytes >= 4 )
				num_chars += 2; // final glyph > 0xFFFF
			else
				++num_chars;
		}
		else
			++num_chars; //wchar_t == UTF-32
		cur += num_bytes;
		i += num_bytes;
	}

	buffer = memory_allocate( HASH_STRING, sizeof( wchar_t ) * ( num_chars + 1 ), 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );

	dest = buffer;
	cur = cstr;
	for( i = 0; i < length; ++i )
	{
		if( !( *cur & 0x80 ) )
			*dest++ = *cur++;
		else
		{
			//Convert through UTF-32
			num_bytes = get_num_bytes_utf8( *cur ) - 1; //Subtract one to get number of _extra_ bytes
			glyph = ( (uint32_t)(*cur) & get_bit_mask( 6 - num_bytes ) ) << ( 6 * num_bytes );
			++cur;
			for( j = 1; j <= num_bytes; ++j, ++cur )
				glyph |= ( (uint32_t)(*cur) & 0x3F ) << ( 6 * ( num_bytes - j ) );
			if( sizeof( wchar_t ) == 2 )
			{
				FOUNDATION_ASSERT( ( glyph < 0xD800 ) || ( glyph > 0xDFFF ) );
				FOUNDATION_ASSERT( glyph <= 0x10FFFF );
				if( ( glyph < 0xD800 ) || ( glyph > 0xDFFF ) )
				{
					if( glyph <= 0xFFFF )
						*dest++ = (uint16_t)glyph;
					else if( glyph <= 0x10FFFF )
					{
						uint32_t val = glyph - 0x10000;
						*dest++ = 0xD800 | ( ( val >> 10 ) & 0x3FF );
						*dest++ = 0xDC00 | (   val         & 0x3FF );
					}
				}
			}
			else
			{
				*dest++ = glyph;
			}
			i += num_bytes;
		}
	}

	*dest = 0;

	return buffer;
}


void wstring_from_string( wchar_t* dest, const char* source, unsigned int max_length )
{
	unsigned int i, j, num;
	uint32_t glyph, val;
	wchar_t* last = dest + ( max_length - 2 );
	const char* cur = source;
	unsigned int length = (unsigned int)strlen( source );
	for( i = 0; ( i < length ) && ( dest < last ); ++i )
	{
		if( !( *cur & 0x80 ) )
			*dest++ = *cur++;
		else
		{
			//Convert through UTF-32
			num = get_num_bytes_utf8( *cur ) - 1; //Subtract one to get number of _extra_ bytes
			glyph = ( (uint32_t)(*cur) & get_bit_mask( 6 - num ) ) << ( 6 * num );
			++cur;
			for( j = 1; j <= num; ++j, ++cur )
				glyph |= ( (uint32_t)(*cur) & 0x3F ) << ( 6 * ( num - j ) );
			if( sizeof( wchar_t ) == 2 )
			{
				FOUNDATION_ASSERT( ( glyph < 0xD800 ) || ( glyph > 0xDFFF ) );
				FOUNDATION_ASSERT( glyph <= 0x10FFFF );
				if( ( glyph < 0xD800 ) || ( glyph > 0xDFFF ) )
				{
					if( glyph <= 0xFFFF )
						*dest++ = (uint16_t)glyph;
					else if( glyph <= 0x10FFFF )
					{
						val = glyph - 0x10000;
						*dest++ = 0xD800 | ( ( val >> 10 ) & 0x3FF );
						*dest++ = 0xDC00 | (   val         & 0x3FF );
					}
				}
			}
			else
			{
				*dest++ = glyph;
			}
			i += num;
		}
	}

	*dest = 0;
}


void wstring_deallocate( wchar_t* str )
{
	memory_deallocate( str );
}


unsigned int wstring_length( const wchar_t* str )
{
	return (unsigned int)wcslen( str );
}


bool wstring_equal( const wchar_t* lhs, const wchar_t* rhs )
{
	return wcscmp( lhs, rhs ) == 0;
}


static unsigned int _string_length_utf16( const uint16_t* p_str )
{
	unsigned int len = 0;
	if( !p_str )
		return 0;
	while( *p_str )
	{
		++len;
		++p_str;
	}
	return len;
}


static unsigned int _string_length_utf32( const uint32_t* p_str )
{
	unsigned int len = 0;
	if( !p_str )
		return 0;
	while( *p_str )
	{
		++len;
		++p_str;
	}
	return len;
}


char* string_allocate_from_wstring( const wchar_t* str, unsigned int length )
{
	if( sizeof( wchar_t ) == 2 )
		return string_allocate_from_utf16( (const uint16_t*)str, length );
	else
		return string_allocate_from_utf32( (const uint32_t*)str, length );
}


char* string_allocate_from_utf16( const uint16_t* str, unsigned int length )
{
	bool swap;
	char* buf;
	unsigned int i, curlen, inlength, maxlen;
	uint32_t glyph, lval;

	maxlen = _string_length_utf16( str );
	if( !length )
		length = maxlen;
	else
		length = ( length < maxlen ) ? length : maxlen;

	inlength = length;
	curlen = 0;

	swap = false;
	for( i = 0; i < inlength; ++i )
	{
		glyph = str[i];
		if( ( glyph == 0xFFFE ) || ( glyph == 0xFEFF ) )
		{
			swap = ( glyph != 0xFEFF );
			continue; //BOM
		}
		if( swap )
			glyph = byteorder_swap16( (uint16_t)glyph );
		if( ( glyph >= 0xD800 ) && ( glyph <= 0xDFFF ) )
		{
			++i;
			lval = str[i];
			if( swap )
				lval = byteorder_swap16( (uint16_t)lval );
			glyph = ( ( ( ( glyph & 0x3FF ) << 10 ) | ( lval & 0x3FF ) ) + 0x10000 );
		}

		curlen += get_num_bytes_as_utf8( glyph );
	}

	buf = memory_allocate( HASH_STRING, ( curlen + 1 ), 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );

	string_convert_utf16( buf, str, curlen + 1, inlength );

	return buf;
}


char* string_allocate_from_utf32( const uint32_t* str, unsigned int length )
{
	bool swap;
	char* buf;
	unsigned int i, curlen, inlength, maxlen;
	uint32_t glyph;

	maxlen = _string_length_utf32( str );
	if( !length )
		length = maxlen;
	else
		length = ( length < maxlen ) ? length : maxlen;

	inlength = length;
	curlen = 0;

	swap = false;
	for( i = 0; i < inlength; ++i )
	{
		glyph = str[i];
		if( ( glyph == 0x0000FEFF ) || ( glyph == 0xFFFE0000 ) )
		{
			swap = ( glyph != 0x0000FEFF );
			continue; //BOM
		}
		if( swap )
			glyph = byteorder_swap32( glyph );
		curlen += get_num_bytes_as_utf8( glyph );
	}

	buf = memory_allocate( HASH_STRING, ( curlen + 1 ), 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );

	string_convert_utf32( buf, str, curlen + 1, inlength );

	return buf;
}


void string_convert_utf16( char* dst, const uint16_t* src, unsigned int dstsize, unsigned int srclength )
{
	bool swap = false;
	uint32_t glyph, lval;
	unsigned int curlen = 0, numbytes = 0;
	unsigned int i;

	for( i = 0; ( i < srclength ) && ( curlen < dstsize ); ++i )
	{
		//Convert through full UTF-32
		glyph = src[i];
		if( ( glyph == 0xFFFE ) || ( glyph == 0xFEFF ) )
		{
			swap = ( glyph != 0xFEFF );
			continue; //BOM
		}
		if( swap )
			glyph = byteorder_swap16( (uint16_t)glyph );
		if( ( glyph >= 0xD800 ) && ( glyph <= 0xDFFF ) )
		{
			++i;
			lval = src[i];
			if( swap )
				lval = byteorder_swap16( (uint16_t)lval );
			glyph = ( ( ( ( glyph & 0x3FF ) << 10 ) | ( lval & 0x3FF ) ) + 0x10000 );
		}

		numbytes = get_num_bytes_as_utf8( glyph );
		if( ( curlen + numbytes ) < dstsize )
			curlen += encode_utf8( dst + curlen, glyph );
	}

	dst[curlen] = 0;
}


void string_convert_utf32( char* dst, const uint32_t* src, unsigned int dstsize, unsigned int srclength )
{
	bool swap = false;
	uint32_t glyph;
	unsigned int curlen = 0, numbytes = 0;
	unsigned int i;

	swap = false;
	for( i = 0; ( i < srclength ) && ( curlen < dstsize ); ++i )
	{
		glyph = src[i];
		if( ( glyph == 0x0000FEFF ) || ( glyph == 0xFFFE0000 ) )
		{
			swap = ( glyph != 0x0000FEFF );
			continue; //BOM
		}
		if( swap )
			glyph = byteorder_swap32( glyph );

		numbytes = get_num_bytes_as_utf8( glyph );
		if( ( curlen + numbytes ) < dstsize )
			curlen += encode_utf8( dst + curlen, glyph );
	}

	dst[curlen] = 0;
}


FOUNDATION_DECLARE_THREAD_LOCAL_ARRAY( char, convert_buffer, 128 )


char* string_from_int( int64_t val, unsigned int width, char fill )
{
	char buf[32];
	return string_clone( string_from_int_buffer( buf, val, width, fill ) );
}


char* string_from_int_buffer( char* buffer, int64_t val, unsigned int width, char fill )
{
	unsigned int len = (unsigned int)sprintf( buffer, "%" PRId64, val );
	if( len < width )
	{
		memmove( buffer + ( width - len ), buffer, len + 1 );
		memset( buffer, fill, width - len );
	}
	return buffer;
}


const char* string_from_int_static( int64_t val, unsigned int width, char fill )
{
	return string_from_int_buffer( get_thread_convert_buffer(), val, width, fill );
}


char* string_from_uint( uint64_t val, bool hex, unsigned int width, char fill )
{
	char buf[32];
	return string_clone( string_from_uint_buffer( buf, val, hex, width, fill ) );
}


char* string_from_uint_buffer( char* buffer, uint64_t val, bool hex, unsigned int width, char fill )
{
	unsigned int len = (unsigned int)sprintf( buffer, hex ? "%" PRIx64 : "%" PRIu64, val );
	if( len < width )
	{
		memmove( buffer + ( width - len ), buffer, len + 1 );
		memset( buffer, fill, width - len );
	}
	return buffer;
}


const char* string_from_uint_static( uint64_t val, bool hex, unsigned int width, char fill )
{
	return string_from_uint_buffer( get_thread_convert_buffer(), val, hex, width, fill );
}


char* string_from_uint128( const uint128_t val )
{
	char buf[34];
	return string_clone( string_from_uint128_buffer( buf, val ) );
}


char* string_from_uint128_buffer( char* buffer, const uint128_t val )
{
	/*unsigned int len = (unsigned int)*/sprintf( buffer, "%016" PRIx64 "%016" PRIx64, val.word[0], val.word[1] );
	return buffer;
}


const char* string_from_uint128_static( const uint128_t val )
{
	return string_from_uint128_buffer( get_thread_convert_buffer(), val );
}


char* string_from_real( real val, unsigned int precision, unsigned int width, char fill )
{
	char str[64];
	string_from_real_buffer( str, val, precision, width, fill );
	return string_clone( str );
}


char* string_from_real_buffer( char* buffer, real val, unsigned int precision, unsigned int width, char fill )
{
	unsigned int len;
#if FOUNDATION_SIZE_REAL == 64
	if( precision )
		len = (unsigned int)sprintf( buffer, "%.*lf", precision, val );
	else
		len = (unsigned int)sprintf( buffer, "%.16lf", val );
#else
	if( precision )
		len = (unsigned int)sprintf( buffer, "%.*f", precision, val );
	else
		len = (unsigned int)sprintf( buffer, "%.7f", val );
#endif
	FOUNDATION_ASSERT( len > 0 && len < 64 );
	{
		unsigned int end = string_find_last_not_of( buffer, "0", STRING_NPOS );
		if( end != STRING_NPOS )
		{
			if( buffer[ end ] == '.' )
				--end;
			if( end != ( len - 1 ) )
			{
				++end;
				len = end;
				buffer[ end ] = 0;
			}
		}
	}
	if( len < width )
	{
		memmove( buffer + ( width - len ), buffer, len + 1 );
		memset( buffer, fill, width - len );
	}

	//Some cleanups
	if( string_equal( buffer, "-0" ) )
	{
		buffer[0] = '0';
		buffer[1] = 0;
	}

	return buffer;
}


const char* string_from_real_static( real val, unsigned int precision, unsigned int width, char fill )
{
	return string_from_real_buffer( get_thread_convert_buffer(), val, precision, width, fill );
}


char* string_from_time( uint64_t t )
{
	char buf[64];
	return string_clone( string_from_time_buffer( buf, t ) );
}


char* string_from_time_buffer( char* buffer, uint64_t t )
{
#if FOUNDATION_PLATFORM_WINDOWS
	time_t timet = t / 1000ULL;
	buffer[0] = 0;
	_ctime64_s( buffer, 64, &timet );
	return string_strip( buffer, STRING_WHITESPACE );
#elif FOUNDATION_PLATFORM_ANDROID
	time_t ts = t / 1000ULL;
	strcpy( buffer, ctime( &ts ) );
	return string_strip( buffer, STRING_WHITESPACE );
#elif FOUNDATION_PLATFORM_POSIX
	buffer[0] = 0;
	time_t ts = (time_t)( t / 1000ULL );
	ctime_r( &ts, buffer );
	return string_strip( buffer, STRING_WHITESPACE );
#else
# error Not implemented
#endif
}


const char* string_from_time_static( uint64_t t )
{
	return string_from_time_buffer( get_thread_convert_buffer(), t );
}


char* string_from_uuid( const uuid_t val )
{
	char buf[38];
	return string_clone( string_from_uuid_buffer( buf, val ) );
}


const char* string_from_uuid_static( const uuid_t val )
{
	return string_from_uuid_buffer( get_thread_convert_buffer(), val );
}


char* string_from_version( const version_t version )
{
	char buf[128];
	return string_clone( string_from_version_buffer( buf, version ) );
}


char* string_from_version_buffer( char* buffer, const version_t version )
{
	if( version.sub.control )
		sprintf( buffer, "%u.%u.%u-%u-%x", (uint32_t)version.sub.major, (uint32_t)version.sub.minor, version.sub.revision, version.sub.build, version.sub.control );
	else if( version.sub.build )
		sprintf( buffer, "%u.%u.%u-%u", (uint32_t)version.sub.major, (uint32_t)version.sub.minor, version.sub.revision, version.sub.build );
	else
		sprintf( buffer, "%u.%u.%u", (uint32_t)version.sub.major, (uint32_t)version.sub.minor, version.sub.revision );
	return buffer;
}


const char* string_from_version_static( const version_t version )
{
	return string_from_version_buffer( get_thread_convert_buffer(), version );
}



int string_to_int( const char* val )
{
	int ret = 0;
	if( val )
		sscanf( val, "%d", &ret );
	return ret;
}


unsigned int string_to_uint( const char* val, bool hex )
{
	unsigned int ret = 0;
	if( val )
		sscanf( val, hex ? "%x" : "%u", &ret );
	return ret;
}


int64_t string_to_int64( const char* val )
{
	int64_t ret = 0;
	if( val )
		sscanf( val, "%" PRId64, &ret );
	return ret;
}


uint64_t string_to_uint64( const char* val, bool hex )
{
	uint64_t ret = 0;
	if( val )
		sscanf( val, hex ? "%" PRIx64 : "%" PRIu64, &ret );
	return ret;
}


uint128_t string_to_uint128( const char* val )
{
	uint128_t ret = uint128_null();
	if( val )
		sscanf( val, "%016" PRIx64 "%016" PRIx64, &ret.word[0], &ret.word[1] );
	return ret;
}


float32_t string_to_float32( const char* val )
{
	float32_t ret = 0.0f;
	if( val )
		sscanf( val, "%f", &ret );
	return ret;
}


float64_t string_to_float64( const char* val )
{
	float64_t ret = 0.0;
	if( val )
		sscanf( val, "%lf", &ret );
	return ret;
}


real string_to_real( const char* val )
{
#if ( FOUNDATION_PLATFORM_LINUX || FOUNDATION_PLATFORM_APPLE ) && ( FOUNDATION_SIZE_REAL == 64 )
	long double ret = 0.0f;
#else
	real ret = 0.0f;
#endif
	if( val )
		sscanf( val, "%" PRIREAL, &ret );
	return ret;
}


version_t string_to_version( const char* val )
{
	//%u.%u.%u-%u.%u
	uint32_t num[5];
	int i;
	for( i = 0; i < 5; ++i )
	{
		num[i] = 0;
		if( val && *val )
		{
			sscanf( val, i < 4 ? "%u" : "%x", num + i );
			while( *val && ( ( *val >= '0' ) && ( *val < '9' ) ) ) val++;
			while( *val && ( ( *val  < '0' ) || ( *val > '9' ) ) ) val++;
		}
	}
	return version_make( num[0], num[1], num[2], num[3], num[4] );
}

