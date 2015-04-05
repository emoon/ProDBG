/* regex.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#include <stdarg.h>


#define REGEXERR_OK                    0
#define REGEXERR_TOO_LONG             -1
#define REGEXERR_MISMATCHED_CAPTURES  -2
#define REGEXERR_MISMATCHED_BLOCKS    -3
#define REGEXERR_INVALID_QUANTIFIER   -4

#define REGEXRES_INTERNAL_FAILURE     -2
#define REGEXRES_NOMATCH              -1
#define REGEXRES_MATCH                 0

#define REGEXCODE_NULL                 (int16_t)0x0000
#define REGEXCODE_WHITESPACE           (int16_t)0x0100
#define REGEXCODE_NONWHITESPACE        (int16_t)0x0200
#define REGEXCODE_DIGIT                (int16_t)0x0300
#define REGEXCODE_NONDIGIT             (int16_t)0x0400


typedef enum
{
	REGEXOP_BEGIN_CAPTURE = 0,
	REGEXOP_END_CAPTURE,
	REGEXOP_BEGINNING_OF_LINE,
	REGEXOP_END_OF_LINE,
	REGEXOP_EXACT_MATCH,
	REGEXOP_META_MATCH,
	REGEXOP_ANY,
	REGEXOP_ANY_OF,
	REGEXOP_ANY_BUT,
	REGEXOP_ZERO_OR_MORE,
	REGEXOP_ONE_OR_MORE,
	REGEXOP_ZERO_OR_MORE_SHORTEST,
	REGEXOP_ONE_OR_MORE_SHORTEST,
	REGEXOP_ZERO_OR_ONE,
	REGEXOP_BRANCH,
	REGEXOP_BRANCH_END
} regex_op_t;

/*static const char* opname[] = {
	"BEGIN_CAPTURE",
	"END_CAPTURE",
	"BEGINNING_OF_LINE",
	"END_OF_LINE",
	"EXACT_MATCH",
	"META_MATCH",
	"ANY",
	"ANY_OF",
	"ANY_BUT",
	"ZERO_OR_MORE",
	"ONE_OR_MORE",
	"ZERO_OR_MORE_SHORTEST",
	"ONE_OR_MORE_SHORTEST",
	"ZERO_OR_ONE",
	"BRANCH",
	"BRANCH_END"
};*/

struct regex_context_t
{
	int             op;
	int             inoffset;
};
typedef struct regex_context_t regex_context_t;


static const char* REGEX_META_CHARACTERS = "^$()[].*+?|\\";


static regex_context_t _regex_execute_single( regex_t* regex, int op, const char* input, int inoffset, int inlength, regex_capture_t* captures, int maxcaptures );
static regex_context_t _regex_execute( regex_t* regex, int op, const char* input, int inoffset, int inlength, regex_capture_t* captures, int maxcaptures );


static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL regex_context_t _regex_context_nomatch( int next_op ) { const regex_context_t context = { next_op, REGEXRES_NOMATCH }; return context; }
static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL regex_context_t _regex_context_internal_failure( int next_op ) { const regex_context_t context = { next_op, REGEXRES_INTERNAL_FAILURE }; return context; }


static int _regex_emit( regex_t** target, bool allow_grow, int ops, ... )
{
	int iop;
	va_list arglist;

	if( (*target)->code_length + ops >= (*target)->code_allocated )
	{
		int new_allocated;
		if( !allow_grow )
			return REGEXERR_TOO_LONG;

		new_allocated = ( (*target)->code_allocated << 1 ) + ops;
		*target = memory_reallocate( *target, sizeof( regex_t ) + new_allocated, 0, sizeof( regex_t ) + (*target)->code_allocated );
		(*target)->code_allocated = new_allocated;
	}

	va_start( arglist, ops );

	for( iop = 0; iop < ops; ++iop )
		(*target)->code[ (*target)->code_length++ ] = va_arg( arglist, int );

	va_end( arglist );

	return REGEXERR_OK;
}


static int _regex_emit_buffer( regex_t** target, bool allow_grow, int ops, const char* buffer )
{
	int iop;

	if( (*target)->code_length + ops >= (*target)->code_allocated )
	{
		int new_allocated;
		if( !allow_grow )
			return REGEXERR_TOO_LONG;

		new_allocated = ( (*target)->code_allocated << 1 ) + ops;
		*target = memory_reallocate( *target, sizeof( regex_t ) + new_allocated, 0, sizeof( regex_t ) + (*target)->code_allocated );
		(*target)->code_allocated = new_allocated;
	}

	for( iop = 0; iop < ops; ++iop )
		(*target)->code[ (*target)->code_length++ ] = buffer[iop];

	return REGEXERR_OK;
}


static int16_t _regex_encode_escape( char code )
{
	switch( code )
	{
		case 'n': return '\n';
		case 'r': return '\r';
		case 't': return '\t';
		case '0': return REGEXCODE_NULL;
		case 's': return REGEXCODE_WHITESPACE;
		case 'S': return REGEXCODE_NONWHITESPACE;
		case 'd': return REGEXCODE_DIGIT;
		case 'D': return REGEXCODE_NONDIGIT;
		default:  break;
	}
	return code;
}


static bool _regex_match_escape( char c, int16_t code )
{
	switch( code )
	{
		case REGEXCODE_NULL:          return c == 0;
		case REGEXCODE_WHITESPACE:    return string_find( STRING_WHITESPACE, c, 0 ) != STRING_NPOS;
		case REGEXCODE_NONWHITESPACE: return string_find( STRING_WHITESPACE, c, 0 ) == STRING_NPOS;
		case REGEXCODE_DIGIT:         return string_find( "0123456789", c, 0 ) != STRING_NPOS;
		case REGEXCODE_NONDIGIT:      return string_find( "0123456789", c, 0 ) == STRING_NPOS;
		default: break;
	}
	return false;
}


static int _regex_compile_quantifier( regex_t** target, bool allow_grow, int last_offset, int op )
{
	int ret, move_size;
	if( ( (*target)->code[last_offset] == REGEXOP_EXACT_MATCH ) && ( (*target)->code[last_offset + 1] > 1 ) )
	{
		char last_char = (*target)->code[(*target)->code_length - 1];
		if( (*target)->code[(*target)->code_length - 2] == 0 ) //Check if meta char
		{
			if( (*target)->code[last_offset + 1] > 2 )
			{
				(*target)->code[last_offset + 1] -= 2;
				(*target)->code_length -= 2;
				if( ( ret = _regex_emit( target, allow_grow, 5, op, REGEXOP_EXACT_MATCH, 2, 0, last_char ) ) )
					return ret;
				return REGEXERR_OK;
			}
		}
		else
		{
			--(*target)->code[last_offset + 1];
			--(*target)->code_length;
			if( ( ret = _regex_emit( target, allow_grow, 4, op, REGEXOP_EXACT_MATCH, 1, last_char ) ) )
				return ret;
			return REGEXERR_OK;
		}
	}

	move_size = 1 + (*target)->code_length - last_offset;

	if( ( ret = _regex_emit( target, allow_grow, 1, 0 ) ) ) //Make sure we have buffer space
		return ret;

	memmove( (*target)->code + last_offset + 1, (*target)->code + last_offset, move_size );
	(*target)->code[last_offset] = op;

	return REGEXERR_OK;
}


static regex_context_t _regex_consume_longest( regex_t* regex, int op, const char* input, int inoffset, int inlength, regex_capture_t* captures, int maxcaptures )
{
	regex_context_t context = { op, inoffset };
	regex_context_t best_context = { -1, inoffset };
	regex_context_t next_context;

	//TODO: Optimization would be to stack all offsets from execute single, then verify matching
	//      of remaining regex starting with highest saved offset
	while( true )
	{
		context = _regex_execute_single( regex, op, input, context.inoffset, inlength, 0, 0 );
		if( context.inoffset < REGEXRES_MATCH )
			break;
		if( context.op >= (int)regex->code_length )
			return context;

		next_context = _regex_execute( regex, context.op, input, context.inoffset, inlength, 0, 0 );
		if( next_context.inoffset >= REGEXRES_MATCH )
		{
			if( captures )
				_regex_execute( regex, context.op, input, context.inoffset, inlength, captures, maxcaptures );
			best_context = next_context;
		}
	}

	//Make sure we return the next op (_regex_execute_single returns next op even on failure)
	if( best_context.op < 0 )
		best_context.op = context.op;

	return best_context;
}


static regex_context_t _regex_consume_shortest( regex_t* regex, int op, const char* input, int inoffset, int inlength, regex_capture_t* captures, int maxcaptures )
{
	regex_context_t context = { op, inoffset };
	regex_context_t best_context = { -1, inoffset };
	regex_context_t next_context;

	while( true )
	{
		context = _regex_execute_single( regex, op, input, context.inoffset, inlength, 0, 0 );
		if( context.inoffset < REGEXRES_MATCH )
			break;
		if( context.op >= (int)regex->code_length )
			return context;

		next_context = _regex_execute( regex, context.op, input, context.inoffset, inlength, 0, 0 );
		if( next_context.inoffset >= REGEXRES_MATCH )
		{
			if( captures )
				_regex_execute( regex, context.op, input, context.inoffset, inlength, captures, maxcaptures );
			best_context = next_context;
			break;
		}
	}

	//Make sure we return the next op (_regex_execute_single returns next op even on failure)
	if( best_context.op < 0 )
		best_context.op = context.op;

	return best_context;
}


static int _regex_parse( regex_t** target, const char** pattern, bool allow_grow, int level )
{
	int ret = 0;
	int last_offset = (*target)->code_length;
	int branch_begin = (*target)->code_length;
	int branch_op = -1;

	do switch( *(*pattern)++ )
	{
		case '^':
		{
			if( ( ret = _regex_emit( target, allow_grow, 1, REGEXOP_BEGINNING_OF_LINE ) ) )
				return ret;
			break;
		}

		case '$':
		{
			if( ( ret = _regex_emit( target, allow_grow, 1, REGEXOP_END_OF_LINE ) ) )
				return ret;
			break;
		}

		case '(':
		{
			int capture = (*target)->num_captures++;

			last_offset = (*target)->code_length;
			if( ( ret = _regex_emit( target, allow_grow, 2, REGEXOP_BEGIN_CAPTURE, capture ) ) )
				return ret;

			_regex_parse( target, pattern, allow_grow, level + 1 );

			if( *(*pattern - 1) != ')' )
				return REGEXERR_MISMATCHED_CAPTURES;

			if( ( ret = _regex_emit( target, allow_grow, 2, REGEXOP_END_CAPTURE, capture ) ) )
			   return ret;

			break;
		}

		case ')':
		{
			if( branch_op >= 0 )
				(*target)->code[branch_op + 1] = (*target)->code_length - branch_op - 2;
			branch_op = -1;

			if( level == 0 )
				return REGEXERR_MISMATCHED_CAPTURES;
			return REGEXERR_OK;
		}

		case '[':
		{
			char local_buffer[64];
			char* buffer = local_buffer;
			int buffer_len = 0;
			int buffer_maxlen = 64;
			int16_t code;
			bool closed = false;
			int op = REGEXOP_ANY_OF;
			if( **pattern == '^' )
			{
				++(*pattern);
				op = REGEXOP_ANY_BUT;
			}

			last_offset = (*target)->code_length;

			while( !closed && **pattern )
			{
				if( buffer_len >= ( buffer_maxlen - 1 ) )
				{
					char* new_buffer = memory_allocate( 0, buffer_maxlen << 1, 0, MEMORY_TEMPORARY );
					memcpy( new_buffer, buffer, buffer_len );
					if( buffer != local_buffer )
						memory_deallocate( buffer );
					buffer = new_buffer;
					buffer_maxlen <<= 1;
				}

				switch( **pattern )
				{
					case ']':
					{
						_regex_emit( target, allow_grow, 2, op, buffer_len );
						if( buffer_len )
							_regex_emit_buffer( target, allow_grow, buffer_len, buffer );
						buffer_len = 0;
						closed = true;
						break;
					}

					case '\\':
					{
						++*pattern;

						if( ( **pattern >= '0' ) && ( **pattern <= '9' ) && ( *( *pattern + 1 ) >= '0' ) && ( *( *pattern + 1 ) <= '9' ) )
						{
							buffer[buffer_len++] = ( ( **pattern - '0' ) << 8 ) | ( *( *pattern + 1 ) - '0' );
							++(*pattern);
						}
						else
						{
							code = _regex_encode_escape( **pattern );
							if( !code || ( code > 0xFF ) )
							{
								buffer[buffer_len++] = 0;
								buffer[buffer_len++] = ( code >> 8 ) & 0xFF;
							}
							else
							{
								buffer[buffer_len++] = (char)code;
							}
						}
						break;
					}

					default:
					{
						buffer[buffer_len++] = **pattern;
						break;
					}
				}
				++*pattern;
			}
			if( buffer != local_buffer )
				memory_deallocate( buffer );
			if( buffer_len )
				return REGEXERR_MISMATCHED_BLOCKS;
			break;
		}

		case '.':
		{
			last_offset = (*target)->code_length;

			if( ( ret = _regex_emit( target, allow_grow, 1, REGEXOP_ANY ) ) )
				return ret;

			break;
		}

		case '*':
		case '+':
		{
			int quantifier = *(*pattern - 1) == '*' ? REGEXOP_ZERO_OR_MORE : REGEXOP_ONE_OR_MORE;

			if( last_offset < 0 )
				return REGEXERR_INVALID_QUANTIFIER;
			if( ( (*target)->code[last_offset] < REGEXOP_EXACT_MATCH ) || ( (*target)->code[last_offset] > REGEXOP_ANY_BUT ) )
				return REGEXERR_INVALID_QUANTIFIER;

			if( **pattern == '?' )
			{
				quantifier = ( quantifier == REGEXOP_ZERO_OR_MORE ) ? REGEXOP_ZERO_OR_MORE_SHORTEST : REGEXOP_ONE_OR_MORE_SHORTEST;
				++(*pattern);
			}

			if( ( ret = _regex_compile_quantifier( target, allow_grow, last_offset, quantifier ) ) )
				return ret;
			break;
		}

		case '?':
		{
			if( last_offset < 0 )
				return REGEXERR_INVALID_QUANTIFIER;
			if( ( (*target)->code[last_offset] < REGEXOP_EXACT_MATCH ) || ( (*target)->code[last_offset] > REGEXOP_ANY_BUT ) )
				return REGEXERR_INVALID_QUANTIFIER;

			if( ( ret = _regex_compile_quantifier( target, allow_grow, last_offset, REGEXOP_ZERO_OR_ONE ) ) )
				return ret;
			break;
		}

		case '\\':
		{
			int16_t code;

			last_offset = (*target)->code_length;

			if( ( **pattern >= '0' ) && ( **pattern <= '9' ) && ( *( *pattern + 1 ) >= '0' ) && ( *( *pattern + 1 ) <= '9' ) )
			{
				char val = ( ( **pattern - '0' ) << 8 ) | ( *( *pattern + 1 ) - '0' );
				(*pattern) += 2;
				if( ( ret = _regex_emit( target, allow_grow, 2, REGEXOP_META_MATCH, val ) ) )
					return ret;
			}
			else
			{
				code = _regex_encode_escape( *(*pattern)++ );
				if( !code || ( code > 0xFF ) )
				{
					if( ( ret = _regex_emit( target, allow_grow, 3, REGEXOP_META_MATCH, 0, ( code >> 8 ) & 0xFF ) ) )
						return ret;
				}
				else
				{
					if( ( ret = _regex_emit( target, allow_grow, 2, REGEXOP_META_MATCH, code ) ) )
						return ret;
				}
			}
			break;
		}

		case '|':
		{
			int move_size = (*target)->code_length - branch_begin;

			if( ( ret = _regex_emit( target, allow_grow, 4, 0, 0, REGEXOP_BRANCH_END, 0 ) ) ) //Make sure we have buffer space
				return ret;

			memmove( (*target)->code + branch_begin + 2, (*target)->code + branch_begin, move_size );
			(*target)->code[branch_begin] = REGEXOP_BRANCH;
			(*target)->code[branch_begin + 1] = move_size + 2;

			branch_op = (*target)->code_length - 2;
			break;
		}

		default:
		{
			//Exact match
			int matchlen = 0;
			const char* matchstart = --(*pattern);
			while( **pattern && ( string_find( REGEX_META_CHARACTERS, **pattern, 0 ) == STRING_NPOS ) )
			{
				++matchlen;
				++(*pattern);
			}

			last_offset = (*target)->code_length;

			if( ( ret = _regex_emit( target, allow_grow, 2, REGEXOP_EXACT_MATCH, matchlen ) ) )
				return ret;
			if( ( ret = _regex_emit_buffer( target, allow_grow, matchlen, matchstart ) ) )
				return ret;

			break;
		}
	} while( **pattern );

	return REGEXERR_OK;
}


static regex_context_t _regex_execute_single( regex_t* regex, int op, const char* input, int inoffset, int inlength, regex_capture_t* captures, int maxcaptures )
{
	regex_context_t context;
	switch( regex->code[op++] )
	{
		case REGEXOP_BEGIN_CAPTURE:
		{
			int capture = regex->code[op++];
			if( captures && ( capture < maxcaptures ) )
				captures[capture].substring = input + inoffset;
			break;
		}

		case REGEXOP_END_CAPTURE:
		{
			int capture = regex->code[op++];
			if( captures && ( capture < maxcaptures ) )
				captures[capture].length = (int)pointer_diff( input + inoffset, captures[capture].substring );
			break;
		}

		case REGEXOP_BEGINNING_OF_LINE:
		{
			if( inoffset != 0 )
				return _regex_context_nomatch( op );
			break;
		}

		case REGEXOP_END_OF_LINE:
		{
			if( inoffset != inlength )
				return _regex_context_nomatch( op );
			break;
		}

		case REGEXOP_ANY_OF:
		{
			char cin, cmatch;
			int ibuf, buffer_len;

			cin = input[inoffset];
			buffer_len = regex->code[op++];

			if( inoffset >= inlength )
				return _regex_context_nomatch( op + buffer_len );

			for( ibuf = 0; ibuf < buffer_len; ++ibuf )
			{
				cmatch = regex->code[op + ibuf];
				if( !cmatch )
				{
					if( _regex_match_escape( cin, (int16_t)regex->code[op + (++ibuf)] << 8 ) )
						break;
				}
				else if( cin == cmatch )
					break;
			}

			if( ibuf == buffer_len )
				return _regex_context_nomatch( op + buffer_len );

			++inoffset;
			op += buffer_len;

			break;
		}

		case REGEXOP_ANY_BUT:
		{
			char cin, cmatch;
			int ibuf, buffer_len;

			cin = input[inoffset];
			buffer_len = regex->code[op++];

			if( inoffset >= inlength )
				return _regex_context_nomatch( op + buffer_len );

			for( ibuf = 0; ibuf < buffer_len; ++ibuf )
			{
				cmatch = regex->code[op + ibuf];
				if( !cmatch )
				{
					if( _regex_match_escape( cin, (int16_t)regex->code[op + (++ibuf)] << 8 ) )
						return _regex_context_nomatch( op + buffer_len );
				}
				else if( cin == cmatch )
					return _regex_context_nomatch( op + buffer_len );
			}

			++inoffset;
			op += buffer_len;

			break;
		}

		case REGEXOP_ANY:
		{
			if( inoffset < inlength )
			{
				++inoffset;
				break;
			}
			return _regex_context_nomatch( op );
		}

		case REGEXOP_EXACT_MATCH:
		{
			int matchlen = regex->code[op++];
			if( ( matchlen > ( inlength - inoffset ) ) || !string_equal_substr( input + inoffset, (const char*)regex->code + op, matchlen ) )
				return _regex_context_nomatch( op + matchlen );
			op += matchlen;
			inoffset += matchlen;
			break;
		}

		case REGEXOP_META_MATCH:
		{
			char cmatch = regex->code[op++];
			if( !cmatch )
			{
				if( _regex_match_escape( input[inoffset++], (int16_t)regex->code[op++] << 8 ) )
					break;
			}
			else if( input[inoffset++] == cmatch )
				break;

			return _regex_context_nomatch( op );
		}

		case REGEXOP_ZERO_OR_MORE:
		{
			context = _regex_consume_longest( regex, op, input, inoffset, inlength, captures, maxcaptures );

			inoffset = context.inoffset;
			op = context.op;

			break;
		}

		case REGEXOP_ONE_OR_MORE:
		{
			context = _regex_execute_single( regex, op, input, inoffset, inlength, captures, maxcaptures );
			if( context.inoffset < REGEXRES_MATCH )
				return context;

			context = _regex_consume_longest( regex, op, input, context.inoffset, inlength, captures, maxcaptures );

			inoffset = context.inoffset;
			op = context.op;

			break;
		}

		case REGEXOP_ZERO_OR_MORE_SHORTEST:
		{
			context = _regex_consume_shortest( regex, op, input, inoffset, inlength, captures, maxcaptures );

			inoffset = context.inoffset;
			op = context.op;

			break;
		}

		case REGEXOP_ONE_OR_MORE_SHORTEST:
		{
			context = _regex_execute_single( regex, op, input, inoffset, inlength, captures, maxcaptures );
			if( context.inoffset < REGEXRES_MATCH )
				return context;

			context = _regex_consume_shortest( regex, op, input, context.inoffset, inlength, captures, maxcaptures );

			inoffset = context.inoffset;
			op = context.op;

			break;
		}

		case REGEXOP_ZERO_OR_ONE:
		{
			int next_op;

			//Try matching with one, and if that succeeds verify complete match of remaining data
			context = _regex_execute_single( regex, op, input, inoffset, inlength, captures, maxcaptures );
			if( context.inoffset >= REGEXRES_MATCH )
			{
				context = _regex_execute( regex, context.op, input, context.inoffset, inlength, captures, maxcaptures );
				if( context.inoffset >= REGEXRES_MATCH )
				{
					op = context.op;
					inoffset = context.inoffset;
					break; // Match with one
				}
			}

			//Failed, try matching remainder with next op (now stored in context.op by _regex_execute_single above)
			next_op = context.op;
			context = _regex_execute( regex, next_op, input, inoffset, inlength, captures, maxcaptures );
			if( context.inoffset >= REGEXRES_MATCH )
			{
				op = context.op;
				inoffset = context.inoffset;
				break; // Match with one
			}

			return _regex_context_nomatch( next_op );
		}

		case REGEXOP_BRANCH:
		{
			int skip = regex->code[op++];
			context = _regex_execute( regex, op, input, inoffset, inlength, captures, maxcaptures );
			if( context.inoffset >= REGEXRES_MATCH )
			{
				inoffset = context.inoffset;
				op = context.op;
			}
			else
			{
				context = _regex_execute( regex, op + skip, input, inoffset, inlength, captures, maxcaptures );
				inoffset = context.inoffset;
				op = context.op;
			}
			break;
		}

		case REGEXOP_BRANCH_END:
		{
			int skip = regex->code[op++];
			op += skip;
			break;
		}

		default:
		{
			log_errorf( 0, ERROR_INTERNAL_FAILURE, "Regex encountered an unsupported op: %02x", (unsigned int)regex->code[op] );
			return _regex_context_internal_failure( op );
		}
	}

	context.op = op;
	context.inoffset = inoffset;

	return context;
}


static regex_context_t _regex_execute( regex_t* regex, int op, const char* input, int inoffset, int inlength, regex_capture_t* captures, int maxcaptures )
{
	regex_context_t context = { op, inoffset };
	while( context.op < (int)regex->code_length )
	{
		context = _regex_execute_single( regex, context.op, input, context.inoffset, inlength, captures, maxcaptures );
		if( context.inoffset < REGEXRES_MATCH )
			break;
	}
	return context;
}


regex_t* regex_compile( const char* pattern )
{
	unsigned int pattern_length = string_length( pattern );
	regex_t* compiled = memory_allocate( HASH_STRING, sizeof( regex_t ) + pattern_length + 1, 0, MEMORY_PERSISTENT );

	compiled->num_captures = 0;
	compiled->code_length = 0;
	compiled->code_allocated = pattern_length + 1;

	if( _regex_parse( &compiled, &pattern, true, 0 ) == REGEXERR_OK )
		return compiled;

	memory_deallocate( compiled );

	return 0;
}


bool regex_match( regex_t* regex, const char* input, int inlength, regex_capture_t* captures, int maxcaptures )
{
	int iin;

	if( !regex || !regex->code_length )
		return true;

	if( !inlength )
		inlength = string_length( input );

	if( regex->code[0] == REGEXOP_BEGINNING_OF_LINE )
	{
		regex_context_t context = _regex_execute( regex, 0, input, 0, inlength, captures, maxcaptures );
		if( context.inoffset >= REGEXRES_MATCH )
			return true;
	}
	else for( iin = 0; iin < inlength; ++iin )
	{
		regex_context_t context = _regex_execute( regex, 0, input, iin, inlength, captures, maxcaptures );
		if( context.inoffset >= REGEXRES_MATCH )
			return true;
	}

	return false;
}


void regex_deallocate( regex_t* regex )
{
	memory_deallocate( regex );
}

