/* config.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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
#include <foundation/internal.h>


#define CONFIG_SECTION_BUCKETS    7
#define CONFIG_KEY_BUCKETS        11

enum config_value_type_t
{
	CONFIGVALUE_BOOL = 0,
	CONFIGVALUE_INT,
	CONFIGVALUE_REAL,
	CONFIGVALUE_STRING,
	CONFIGVALUE_STRING_CONST,
	CONFIGVALUE_STRING_VAR,
	CONFIGVALUE_STRING_CONST_VAR
};
typedef enum config_value_type_t config_value_type_t;

struct config_key_t
{
	hash_t                  name;
	config_value_type_t     type;
	bool                    bval;
	int64_t                 ival;
	char*                   sval;
	char*                   expanded;
	real                    rval;
};
typedef FOUNDATION_ALIGN(8) struct config_key_t config_key_t;

struct config_section_t
{
	hash_t                  name;
	config_key_t*           key[CONFIG_KEY_BUCKETS];
};
typedef FOUNDATION_ALIGN(8) struct config_section_t config_section_t;

FOUNDATION_STATIC_ASSERT( FOUNDATION_ALIGNOF( config_key_t ) == 8, "config_key_t alignment" );
FOUNDATION_STATIC_ASSERT( FOUNDATION_ALIGNOF( config_section_t ) == 8, "config_section_t alignment" );

//Global config store
static config_section_t* _config_section[CONFIG_SECTION_BUCKETS];


static int64_t _config_string_to_int( const char* str )
{
	unsigned int length = string_length( str );
	unsigned int first_nonnumeric;
	unsigned int dot_position;
	if( length < 2 )
		return string_to_int64( str );

	first_nonnumeric = string_find_first_not_of( str, "0123456789.", 0 );
	if( ( first_nonnumeric == ( length - 1 ) ) && ( ( str[ first_nonnumeric ] == 'm' ) || ( str[ first_nonnumeric ] == 'M' ) ) )
	{
		dot_position = string_find( str, '.', 0 );
		if( dot_position != STRING_NPOS )
		{
			if( string_find( str, '.', dot_position + 1 ) == STRING_NPOS )
				return (int64_t)( string_to_float64( str ) * ( 1024.0 * 1024.0 ) );
			return string_to_int64( str ); //More than one dot
		}
		return string_to_int64( str ) * ( 1024LL * 1024LL );
	}
	if( ( first_nonnumeric == ( length - 1 ) ) && ( ( str[ first_nonnumeric ] == 'k' ) || ( str[ first_nonnumeric ] == 'K' ) ) )
	{
		dot_position = string_find( str, '.', 0 );
		if( dot_position != STRING_NPOS )
		{
			 if( string_find( str, '.', dot_position + 1 ) == STRING_NPOS )
				return (int64_t)( string_to_float64( str ) * 1024.0 );
			 return string_to_int64( str ); //More than one dot
		}
		return string_to_int64( str ) * 1024LL;
	}

	return string_to_int64( str );
}


static real _config_string_to_real( const char* str )
{
	unsigned int length = string_length( str );
	unsigned int first_nonnumeric;
	unsigned int dot_position;
	if( length < 2 )
		return string_to_real( str );

	first_nonnumeric = string_find_first_not_of( str, "0123456789.", 0 );
	if( ( first_nonnumeric == ( length - 1 ) ) && ( ( str[ first_nonnumeric ] == 'm' ) || ( str[ first_nonnumeric ] == 'M' ) ) )
	{
		dot_position = string_find( str, '.', 0 );
		if( dot_position != STRING_NPOS )
		{
			if( string_find( str, '.', dot_position + 1 ) != STRING_NPOS )
				return string_to_real( str ); //More than one dot
		}
		return string_to_real( str ) * ( REAL_C( 1024.0 ) * REAL_C( 1024.0 ) );
	}
	if( ( first_nonnumeric == ( length - 1 ) ) && ( ( str[ first_nonnumeric ] == 'k' ) || ( str[ first_nonnumeric ] == 'K' ) ) )
	{
		dot_position = string_find( str, '.', 0 );
		if( dot_position != STRING_NPOS )
		{
			if( string_find( str, '.', dot_position + 1 ) != STRING_NPOS )
				return string_to_real( str ); //More than one dot
		}
		return string_to_real( str ) * REAL_C( 1024.0 );
	}

	return string_to_real( str );
}


static FOUNDATION_NOINLINE const char* _expand_environment( hash_t key, char* var )
{
	if( key == HASH_EXECUTABLE_NAME )
		return environment_executable_name();
	else if( key == HASH_EXECUTABLE_DIRECTORY )
		return environment_executable_directory();
	else if( key == HASH_EXECUTABLE_PATH )
		return environment_executable_path();
	else if( key == HASH_INITIAL_WORKING_DIRECTORY )
		return environment_initial_working_directory();
	else if( key == HASH_CURRENT_WORKING_DIRECTORY )
		return environment_current_working_directory();
	else if( key == HASH_HOME_DIRECTORY )
		return environment_home_directory();
	else if( key == HASH_TEMPORARY_DIRECTORY )
		return environment_temporary_directory();
	else if( string_equal_substr( var, "variable[", 9 ) )  //variable[varname] - Environment variable named "varname"
	{
		const char* value;
		unsigned int end_pos = string_find( var, ']', 9 );
		if( end_pos != STRING_NPOS )
			var[end_pos] = 0;
		value = environment_variable( var + 9 );
		if( end_pos != STRING_NPOS )
			var[end_pos] = ']';
		return value;
	}
	return "";
}


static FOUNDATION_NOINLINE char* _expand_string( hash_t section_current, char* str )
{
	char* expanded;
	char* variable;
	unsigned int var_pos, var_end_pos, variable_length, separator, var_offset;
	hash_t section, key;

	expanded = str;
	var_pos = string_find_string( expanded, "$(", 0 );

	while( var_pos != STRING_NPOS )
	{
		var_end_pos = string_find( expanded, ')', var_pos + 2 );
		FOUNDATION_ASSERT_MSG( var_end_pos != STRING_NPOS, "Malformed config variable statement" );
		variable = string_substr( expanded, var_pos, ( var_end_pos != STRING_NPOS ) ? ( 1 + var_end_pos - var_pos ) : STRING_NPOS );

		section = section_current;
		key = 0;
		variable_length = string_length( variable );
		separator = string_find( variable, ':', 0 );
		if( separator != STRING_NPOS )
		{
			if( separator != 2 )
				section = hash( variable + 2, separator - 2 );
			var_offset = separator + 1;
		}
		else
		{
			var_offset = 2;
		}
		key = hash( variable + var_offset, variable_length - ( var_offset + ( variable[ variable_length - 1 ] == ')' ? 1 : 0 ) ) );

		if( expanded == str )
			expanded = string_clone( str );

		if( section != HASH_ENVIRONMENT )
			expanded = string_replace( expanded, variable, config_string( section, key ), false );
		else
			expanded = string_replace( expanded, variable, _expand_environment( key, variable + var_offset ), false );
		string_deallocate( variable );

		var_pos = string_find_string( expanded, "$(", 0 );
	}
#if BUILD_ENABLE_CONFIG_DEBUG
	if( str != expanded )
		log_debugf( HASH_CONFIG, "Expanded config value \"%s\" to \"%s\"", str, expanded );
#endif

	return expanded;
}


static FOUNDATION_NOINLINE void _expand_string_val( hash_t section, config_key_t* key )
{
	bool is_true;
	FOUNDATION_ASSERT( key->sval );
	if( key->expanded != key->sval )
		string_deallocate( key->expanded );
	key->expanded = _expand_string( section, key->sval );

	is_true = string_equal( key->expanded, "true" );
	key->bval = ( string_equal( key->expanded, "false" ) || string_equal( key->expanded, "0" ) || !string_length( key->expanded ) ) ? false : true;
	key->ival = is_true ? 1 : _config_string_to_int( key->expanded );
	key->rval = is_true ? REAL_C(1.0) : _config_string_to_real( key->expanded );
}



int _config_initialize( void )
{
	config_load( "foundation", 0ULL, true, false );
	config_load( "application", 0ULL, true, false );

	//Load per-user config
	config_load( "user", HASH_USER, false, true );

	return 0;
}


void _config_shutdown( void )
{
	int isb, is, ikb, ik, ssize, ksize;
	config_section_t* section;
	config_key_t* key;
	for( isb = 0; isb < CONFIG_SECTION_BUCKETS; ++isb )
	{
		section = _config_section[isb];
		for( is = 0, ssize = array_size( section ); is < ssize; ++is )
		{
			for( ikb = 0; ikb < CONFIG_KEY_BUCKETS; ++ikb )
			{
				/*lint -e{613} array_size( section ) in loop condition does the null pointer guard */
				key = section[is].key[ikb];
				for( ik = 0, ksize = array_size( key ); ik < ksize; ++ik )
				{
					/*lint --e{613} array_size( key ) in loop condition does the null pointer guard */
					if( key[ik].expanded != key[ik].sval )
						string_deallocate( key[ik].expanded );
					if( ( key[ik].type != CONFIGVALUE_STRING_CONST ) && ( key[ik].type != CONFIGVALUE_STRING_CONST_VAR ) )
						string_deallocate( key[ik].sval );
				}
				array_deallocate( key );
			}
		}
		array_deallocate( section );
	}
}


void config_load( const char* name, hash_t filter_section, bool built_in, bool overwrite )
{
	/*lint --e{838} Safety null assign all pointers for all preprocessor paths */
	/*lint --e{750} Unused macros in some paths */
#define NUM_SEARCH_PATHS 10
#define ANDROID_ASSET_PATH_INDEX 5
	char* sub_exe_path = 0;
	char* exe_parent_path = 0;
	char* exe_processed_path = 0;
	char* abs_exe_parent_path = 0;
	char* abs_exe_processed_path = 0;
	char* bundle_path = 0;
	char* home_dir = 0;
	char* cmdline_path = 0;
	char* cwd_config_path = 0;
	const char* paths[NUM_SEARCH_PATHS];
#if !FOUNDATION_PLATFORM_FAMILY_MOBILE && !FOUNDATION_PLATFORM_PNACL
	const char* const* cmd_line;
	int icl, clsize;
#endif
	int start_path, i, j;

	const char buildsuffix[4][9] = { "/debug", "/release", "/profile", "/deploy" };
	const char platformsuffix[PLATFORM_INVALID+1][14] = { "/win32", "/win64", "/osx", "/ios", "/android", "/raspberrypi", "/pnacl", "/bsd", "/unknown" };
	const char binsuffix[1][5] = { "/bin" };

	FOUNDATION_ASSERT( name );

	sub_exe_path = path_merge( environment_executable_directory(), "config" );
	exe_parent_path = path_merge( environment_executable_directory(), "../config" );
	abs_exe_parent_path = path_make_absolute( exe_parent_path );

	exe_processed_path = string_clone( environment_executable_directory() );
	for( i = 0; i < 4; ++i )
	{
		if( string_ends_with( exe_processed_path, buildsuffix[i] ) )
		{
			exe_processed_path[ string_length( exe_processed_path ) - string_length( buildsuffix[i] ) ] = 0;
			break;
		}
	}
	for( i = 0; i < 7; ++i )
	{
		if( string_ends_with( exe_processed_path, platformsuffix[i] ) )
		{
			exe_processed_path[ string_length( exe_processed_path ) - string_length( platformsuffix[i] ) ] = 0;
			break;
		}
	}
	for( i = 0; i < 1; ++i )
	{
		if( string_ends_with( exe_processed_path, binsuffix[i] ) )
		{
			exe_processed_path[ string_length( exe_processed_path ) - string_length( binsuffix[i] ) ] = 0;
			break;
		}
	}
	exe_processed_path = path_append( exe_processed_path, "config" );
	abs_exe_processed_path = path_make_absolute( exe_processed_path );

	paths[0] = environment_executable_directory();
#if FOUNDATION_PLATFORM_PNACL
	paths[1] = 0;
	paths[2] = 0;
#else
	paths[1] = sub_exe_path;
	paths[2] = abs_exe_parent_path;
#endif
	paths[3] = abs_exe_processed_path;

#if FOUNDATION_PLATFORM_FAMILY_DESKTOP && !BUILD_DEPLOY
	paths[4] = environment_initial_working_directory();
#else
	paths[4] = 0;
#endif

#if FOUNDATION_PLATFORM_MACOSX
	bundle_path = path_merge( environment_executable_directory(), "../Resources/config" );
	paths[5] = bundle_path;
#elif FOUNDATION_PLATFORM_IOS
	bundle_path = path_merge( environment_executable_directory(), "config" );
	paths[5] = bundle_path;
#elif FOUNDATION_PLATFORM_ANDROID
	paths[5] = "/config";
#else
	paths[5] = 0;
#endif

#if FOUNDATION_PLATFORM_FAMILY_DESKTOP
	paths[6] = environment_current_working_directory();
#else
	paths[6] = 0;
#endif

	paths[7] = 0;
	paths[8] = 0;

	string_deallocate( exe_parent_path );
	string_deallocate( exe_processed_path );

#if FOUNDATION_PLATFORM_FAMILY_DESKTOP
	cwd_config_path = path_merge( environment_current_working_directory(), "config" );
	paths[7] = cwd_config_path;

	cmd_line = environment_command_line();
	/*lint -e{850} We modify loop var to skip extra arg */
	for( icl = 0, clsize = array_size( cmd_line ); icl < clsize; ++icl )
	{
		/*lint -e{613} array_size( cmd_line ) in loop condition does the null pointer guard */
		if( string_equal_substr( cmd_line[icl], "--configdir", 11 ) )
		{
			if( string_equal_substr( cmd_line[icl], "--configdir=", 12 ) )
			{
				paths[8] = cmdline_path = string_substr( cmd_line[icl], 12, STRING_NPOS );
			}
			else if( icl < ( clsize - 1 ) )
			{
				paths[8] = cmdline_path = string_clone( cmd_line[++icl] );
			}
		}
	}
#endif

	start_path = 0;

	paths[9] = 0;
	if( !built_in )
	{
#if FOUNDATION_PLATFORM_WINDOWS
		home_dir = path_merge( environment_home_directory(), environment_application()->config_dir );
#elif FOUNDATION_PLATFORM_LINUX || FOUNDATION_PLATFORM_MACOSX || FOUNDATION_PLATFORM_BSD
		home_dir = path_prepend( string_concat( ".", environment_application()->config_dir ), environment_home_directory() );
#endif
		if( home_dir )
			paths[9] = home_dir;
		start_path = 9;
	}

	for( i = start_path; i < NUM_SEARCH_PATHS; ++i )
	{
		char* filename;
		stream_t* istream;
		bool path_already_searched = false;

		if( !paths[i] )
			continue;

		for( j = start_path; j < i; ++j )
		{
			if( paths[j] && string_equal( paths[j], paths[i] ) )
			{
				path_already_searched = true;
				break;
			}
		}
		if( path_already_searched )
			continue;

		//TODO: Support loading configs from virtual file system (i.e in zip/other packages)
		filename = string_append( path_merge( paths[i], name ), ".ini" );
		istream = 0;
#if FOUNDATION_PLATFORM_ANDROID
		if( i == ANDROID_ASSET_PATH_INDEX )
			istream = asset_stream_open( filename, STREAM_IN );
		else
#endif
		istream = stream_open( filename, STREAM_IN );
		if( istream )
		{
			config_parse( istream, filter_section, overwrite );
			stream_deallocate( istream );
		}
		string_deallocate( filename );

		if( built_in )
		{
			const char* FOUNDATION_PLATFORM_name =
#if FOUNDATION_PLATFORM_WINDOWS
				"windows";
#elif FOUNDATION_PLATFORM_LINUX_RASPBERRYPI
				"raspberrypi";
#elif FOUNDATION_PLATFORM_LINUX
				"linux";
#elif FOUNDATION_PLATFORM_MACOSX
				"osx";
#elif FOUNDATION_PLATFORM_IOS
				"ios";
#elif FOUNDATION_PLATFORM_ANDROID
				"android";
#elif FOUNDATION_PLATFORM_PNACL
				"pnacl";
#elif FOUNDATION_PLATFORM_BSD
				"bsd";
#elif FOUNDATION_PLATFORM_TIZEN
				"tizen";
#else
#  error Insert platform name
				"unknown";
#endif
			filename = string_append( path_append( path_merge( paths[i], FOUNDATION_PLATFORM_name ), name ), ".ini" );
#if FOUNDATION_PLATFORM_ANDROID
			if( i == ANDROID_ASSET_PATH_INDEX )
				istream = asset_stream_open( filename, STREAM_IN );
			else
#endif
			istream = stream_open( filename, STREAM_IN );
			if( istream )
			{
				config_parse( istream, filter_section, overwrite );
				stream_deallocate( istream );
			}
			string_deallocate( filename );
		}
	}

	string_deallocate( home_dir );
	string_deallocate( cmdline_path );
	string_deallocate( sub_exe_path );
	string_deallocate( abs_exe_processed_path );
	string_deallocate( abs_exe_parent_path );
	string_deallocate( bundle_path );
	string_deallocate( cwd_config_path );
}


static FOUNDATION_NOINLINE config_section_t* config_section( hash_t section, bool create )
{
	config_section_t* bucket;
	int ib, bsize;

	bucket = _config_section[ section % CONFIG_SECTION_BUCKETS ];
	for( ib = 0, bsize = array_size( bucket ); ib < bsize; ++ib )
	{
		/*lint --e{613} array_size( bucket ) in loop condition does the null pointer guard */
		if( bucket[ib].name == section )
			return bucket + ib;
	}

	if( !create )
		return 0;

	//TODO: Thread safeness
	{
		config_section_t new_section;
		memset( &new_section, 0, sizeof( new_section ) );
		new_section.name = section;

		array_push_memcpy( bucket, &new_section );
		_config_section[ section % CONFIG_SECTION_BUCKETS ] = bucket;
	}

	return bucket + bsize;
}


static FOUNDATION_NOINLINE config_key_t* config_key( hash_t section, hash_t key, bool create )
{
	config_key_t new_key;
	config_section_t* csection;
	config_key_t* bucket;
	int ib, bsize;

	csection = config_section( section, create );
	if( !csection )
	{
		FOUNDATION_ASSERT( !create );
		return 0;
	}
	bucket = csection->key[ key % CONFIG_KEY_BUCKETS ];
	for( ib = 0, bsize = array_size( bucket ); ib < bsize; ++ib )
	{
		/*lint --e{613} array_size( bucket ) in loop condition does the null pointer guard */
		if( bucket[ib].name == key )
			return bucket + ib;
	}

	if( !create )
		return 0;

	memset( &new_key, 0, sizeof( new_key ) );
	new_key.name = key;

	//TODO: Thread safeness
	array_push_memcpy( bucket, &new_key );
	csection->key[ key % CONFIG_KEY_BUCKETS ] = bucket;

	return bucket + bsize;
}


bool config_bool( hash_t section, hash_t key )
{
	config_key_t* key_val = config_key( section, key, false );
	if( key_val && ( key_val->type >= CONFIGVALUE_STRING_VAR ) )
		_expand_string_val( section, key_val );
	return key_val ? key_val->bval : false;
}


int64_t config_int( hash_t section, hash_t key )
{
	config_key_t* key_val = config_key( section, key, false );
	if( key_val && ( key_val->type >= CONFIGVALUE_STRING_VAR ) )
		_expand_string_val( section, key_val );
	return key_val ? key_val->ival : 0;
}


real config_real( hash_t section, hash_t key )
{
	config_key_t* key_val = config_key( section, key, false );
	if( key_val && ( key_val->type >= CONFIGVALUE_STRING_VAR ) )
		_expand_string_val( section, key_val );
	return key_val ? key_val->rval : 0;
}


const char* config_string( hash_t section, hash_t key )
{
	config_key_t* key_val = config_key( section, key, false );
	if( !key_val )
		return "";
	//Convert to string
	/*lint --e{788} We use default for remaining enums */
	switch( key_val->type )
	{
		case CONFIGVALUE_BOOL:  return key_val->bval ? "true" : "false";
		case CONFIGVALUE_INT:   if( !key_val->sval ) key_val->sval = string_from_int( key_val->ival, 0, 0 ); return key_val->sval;
		case CONFIGVALUE_REAL:  if( !key_val->sval ) key_val->sval = string_from_real( key_val->rval, 4, 0, '0' ); return key_val->sval;
		default: break;
	}
	//String value of some form
	if( !key_val->sval )
		return "";
	if( key_val->type >= CONFIGVALUE_STRING_VAR )
	{
		_expand_string_val( section, key_val );
		return key_val->expanded;
	}
	return key_val->sval;
}


hash_t config_string_hash( hash_t section, hash_t key )
{
	const char* value = config_string( section, key );
	return value ? hash( value, string_length( value ) ) : HASH_EMPTY_STRING;
}


void config_set_bool( hash_t section, hash_t key, bool value )
{
	config_key_t* key_val = config_key( section, key, true );
	if( !FOUNDATION_VALIDATE( key_val ) ) return;
	key_val->bval = value;
	key_val->ival = ( value ? 1 : 0 );
	key_val->rval = ( value ? REAL_C( 1.0 ) : REAL_C( 0.0 ) );
	if( key_val->expanded != key_val->sval )
		string_deallocate( key_val->expanded );
	if( ( key_val->type != CONFIGVALUE_STRING_CONST ) && ( key_val->type != CONFIGVALUE_STRING_CONST_VAR ) )
		string_deallocate( key_val->sval );
	key_val->sval = 0;
	key_val->expanded = 0;
	key_val->type = CONFIGVALUE_BOOL;
}


void config_set_int( hash_t section, hash_t key, int64_t value )
{
	config_key_t* key_val = config_key( section, key, true );
	if( !FOUNDATION_VALIDATE( key_val ) ) return;
	key_val->bval = value ? true : false;
	key_val->ival = value;
	key_val->rval = (real)value;
	if( key_val->expanded != key_val->sval )
		string_deallocate( key_val->expanded );
	if( ( key_val->type != CONFIGVALUE_STRING_CONST ) && ( key_val->type != CONFIGVALUE_STRING_CONST_VAR ) )
		string_deallocate( key_val->sval );
	key_val->sval = 0;
	key_val->expanded = 0;
	key_val->type = CONFIGVALUE_INT;
}


void config_set_real( hash_t section, hash_t key, real value )
{
	config_key_t* key_val = config_key( section, key, true );
	if( !FOUNDATION_VALIDATE( key_val ) ) return;
	key_val->bval = !math_realzero( value );
	key_val->ival = (int64_t)value;
	key_val->rval = value;
	if( key_val->expanded != key_val->sval )
		string_deallocate( key_val->expanded );
	if( ( key_val->type != CONFIGVALUE_STRING_CONST ) && ( key_val->type != CONFIGVALUE_STRING_CONST_VAR ) )
		string_deallocate( key_val->sval );
	key_val->sval = 0;
	key_val->expanded = 0;
	key_val->type = CONFIGVALUE_REAL;
}


void config_set_string( hash_t section, hash_t key, const char* value )
{
	config_key_t* key_val = config_key( section, key, true );
	if( !FOUNDATION_VALIDATE( key_val ) ) return;
	if( key_val->expanded != key_val->sval )
		string_deallocate( key_val->expanded );
	if( ( key_val->type != CONFIGVALUE_STRING_CONST ) && ( key_val->type != CONFIGVALUE_STRING_CONST_VAR ) )
		string_deallocate( key_val->sval );

	key_val->sval = string_clone( value );
	key_val->expanded = 0;
	key_val->type = ( ( string_find_string( key_val->sval, "$(", 0 ) != STRING_NPOS ) ? CONFIGVALUE_STRING_VAR : CONFIGVALUE_STRING );

	if( key_val->type == CONFIGVALUE_STRING )
	{
		bool is_true = string_equal( key_val->sval, "true" );
		key_val->bval = ( string_equal( key_val->sval, "false" ) || string_equal( key_val->sval, "0" ) || !string_length( key_val->sval ) ) ? false : true;
		key_val->ival = is_true ? 1 : _config_string_to_int( key_val->sval );
		key_val->rval = is_true ? REAL_C(1.0) : _config_string_to_real( key_val->sval );
	}
}


void config_set_string_constant( hash_t section, hash_t key, const char* value )
{
	config_key_t* key_val = config_key( section, key, true );
	if( !FOUNDATION_VALIDATE( key_val ) ) return;
	if( !FOUNDATION_VALIDATE( value ) ) return;
	if( key_val->expanded != key_val->sval )
		string_deallocate( key_val->expanded );
	if( ( key_val->type != CONFIGVALUE_STRING_CONST ) && ( key_val->type != CONFIGVALUE_STRING_CONST_VAR ) )
		string_deallocate( key_val->sval );
	//key_val->sval = (char*)value;
	memcpy( &key_val->sval, &value, sizeof( char* ) ); //Yeah yeah, we're storing a const pointer in a non-const var
	key_val->expanded = 0;
	key_val->type = ( ( string_find_string( key_val->sval, "$(", 0 ) != STRING_NPOS ) ? CONFIGVALUE_STRING_CONST_VAR : CONFIGVALUE_STRING_CONST );

	if( key_val->type == CONFIGVALUE_STRING_CONST )
	{
		bool is_true = string_equal( key_val->sval, "true" );
		key_val->bval = ( string_equal( key_val->sval, "false" ) || string_equal( key_val->sval, "0" ) || !string_length( key_val->sval ) ) ? false : true;
		key_val->ival = is_true ? 1 : _config_string_to_int( key_val->sval );
		key_val->rval = is_true ? REAL_C(1.0) : _config_string_to_real( key_val->sval );
	}
}


void config_parse( stream_t* stream, hash_t filter_section, bool overwrite )
{
	char* buffer;
	hash_t section = 0;
	hash_t key = 0;
	unsigned int line = 0;

#if BUILD_ENABLE_CONFIG_DEBUG
	log_debugf( HASH_CONFIG, "Parsing config stream: %s", stream_path( stream ) );
#endif
	buffer = memory_allocate( 0, 1024ULL, 0, MEMORY_TEMPORARY | MEMORY_ZERO_INITIALIZED );
	while( !stream_eos( stream ) )
	{
		++line;
		stream_read_line_buffer( stream, buffer, 1024, '\n' );
		string_strip( buffer, " \t\n\r" );
		if( !string_length( buffer ) || ( buffer[0] == ';' ) || ( buffer[0] == '#' ) )
			continue;
		if( buffer[0] == '[' )
		{
			//Section declaration
			unsigned int endpos = string_rfind( buffer, ']', string_length( buffer ) - 1 );
			if( ( endpos == STRING_NPOS ) || ( endpos < 1 ) )
			{
				log_warnf( HASH_CONFIG, WARNING_BAD_DATA, "Invalid section declaration on line %d in config stream '%s'", line, stream_path( stream ) );
				continue;
			}
			buffer[endpos] = 0;
			section = hash( buffer + 1, endpos - 1 );
#if BUILD_ENABLE_CONFIG_DEBUG
			log_debugf( HASH_CONFIG, "  config: section set to '%s' (0x%llx)", buffer + 1, section );
#endif
		}
		else if( !filter_section || ( filter_section == section ) )
		{
			//name=value declaration
			char* name;
			char* value;
			unsigned int separator = string_find( buffer, '=', 0 );
			if( separator == STRING_NPOS )
			{
				log_warnf( HASH_CONFIG, WARNING_BAD_DATA, "Invalid value declaration on line %d in config stream '%s', missing assignment operator '=': %s", line, stream_path( stream ), buffer );
				continue;
			}

			name = string_strip_substr( buffer, " \t", separator );
			value = string_strip( buffer + separator + 1, " \t" );
			if( !string_length( name ) )
			{
				log_warnf( HASH_CONFIG, WARNING_BAD_DATA, "Invalid value declaration on line %d in config stream '%s', empty name string", line, stream_path( stream ) );
				continue;
			}

			key = hash( name, string_length( name ) );

			if( overwrite || !config_key( section, key, false ) )
			{
#if BUILD_ENABLE_CONFIG_DEBUG
				log_debugf( HASH_CONFIG, "  config: %s (0x%llx) = %s", name, key, value );
#endif

				if( !string_length( value ) )
					config_set_string( section, key, "" );
				else if( string_equal( value, "false" ) )
					config_set_bool( section, key, false );
				else if( string_equal( value, "true" ) )
					config_set_bool( section, key, true );
				else if( ( string_find( value, '.', 0 ) != STRING_NPOS ) && ( string_find_first_not_of( value, "0123456789.", 0 ) == STRING_NPOS ) && ( string_find( value, '.', string_find( value, '.', 0 ) + 1 ) == STRING_NPOS ) ) //Exactly one "."
					config_set_real( section, key, string_to_real( value ) );
				else if( string_find_first_not_of( value, "0123456789", 0 ) == STRING_NPOS )
					config_set_int( section, key, string_to_int64( value ) );
				else
					config_set_string( section, key, value );
			}
		}
	}
	memory_deallocate( buffer );
}


void config_parse_commandline( const char* const* cmdline, unsigned int num )
{
	//TODO: Implement, format --section:key=value
	unsigned int arg;
	for( arg = 0; arg < num; ++arg )
	{
		if( string_match_pattern( cmdline[arg], "--*:*=*" ) )
		{
			unsigned int first_sep = string_find( cmdline[arg], ':', 0 );
			unsigned int second_sep = string_find( cmdline[arg], '=', 0 );
			if( ( first_sep != STRING_NPOS ) && ( second_sep != STRING_NPOS ) && ( first_sep < second_sep ) )
			{
				unsigned int section_length = first_sep - 2;
				unsigned int end_pos = first_sep + 1;
				unsigned int key_length = second_sep - end_pos;

				const char* section_str = cmdline[arg] + 2;
				const char* key_str = pointer_offset_const( cmdline[arg], end_pos );

				hash_t section = hash( section_str, section_length );
				hash_t key = hash( key_str, key_length );

				char* value = string_substr( cmdline[arg], second_sep + 1, STRING_NPOS );
				char* set_value = value;

				unsigned int value_length = string_length( value );

				if( !value_length )
					config_set_string( section, key, "" );
				else if( string_equal( value, "false" ) )
					config_set_bool( section, key, false );
				else if( string_equal( value, "true" ) )
					config_set_bool( section, key, true );
				else if( ( string_find( value, '.', 0 ) != STRING_NPOS ) && ( string_find_first_not_of( value, "0123456789.", 0 ) == STRING_NPOS ) && ( string_find( value, '.', string_find( value, '.', 0 ) + 1 ) == STRING_NPOS ) ) //Exactly one "."
					config_set_real( section, key, string_to_real( value ) );
				else if( string_find_first_not_of( value, "0123456789", 0 ) == STRING_NPOS )
					config_set_int( section, key, string_to_int64( value ) );
				else
				{
					if( ( value_length > 1 ) && ( value[0] == '"' ) && ( value[ value_length - 1 ] == '"' ) )
					{
						value[ value_length - 1 ] = 0;
						set_value = value + 1;
						config_set_string( section, key, set_value );
					}
					else
					{
						config_set_string( section, key, value );
					}
				}

				log_infof( HASH_CONFIG, "Config value from command line: %.*s:%.*s = %s", section_length, section_str, key_length, key_str, set_value );

				string_deallocate( value );
			}
		}
	}
}


void config_write( stream_t* stream, hash_t filter_section, const char* (*string_mapper)( hash_t ) )
{
	config_section_t* csection;
	config_key_t* bucket;
	int key, ib, bsize;

	stream_set_binary( stream, false );

	//TODO: If random access stream, update section if available, else append at end of stream
	//if( stream_is_sequential( stream ) )
	{
		stream_write_format( stream, "[%s]", string_mapper( filter_section ) );
		stream_write_endl( stream );

		csection = config_section( filter_section, false );
		if( csection ) for( key = 0; key < CONFIG_KEY_BUCKETS; ++key )
		{
			bucket = csection->key[ key ];
			if( bucket ) for( ib = 0, bsize = array_size( bucket ); ib < bsize; ++ib )
			{
				stream_write_format( stream, "\t%s\t\t\t\t= ", string_mapper( bucket[ib].name ) );
				switch( bucket[ib].type )
				{
					case CONFIGVALUE_BOOL:
						stream_write_bool( stream, bucket[ib].bval );
						break;

					case CONFIGVALUE_INT:
						stream_write_int64( stream, bucket[ib].ival );
						break;

					case CONFIGVALUE_REAL:
#if FOUNDATION_SIZE_REAL == 64
						stream_write_float64( stream, bucket[ib].rval );
#else
						stream_write_float32( stream, bucket[ib].rval );
#endif
						break;

					case CONFIGVALUE_STRING:
					case CONFIGVALUE_STRING_CONST:
					case CONFIGVALUE_STRING_VAR:
					case CONFIGVALUE_STRING_CONST_VAR:
						stream_write_string( stream, bucket[ib].sval );
						break;

					default:
						break;
				}
				stream_write_endl( stream );
			}
		}
	}
}

