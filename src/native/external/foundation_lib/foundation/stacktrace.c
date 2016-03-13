/* stacktrace.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


#if FOUNDATION_PLATFORM_WINDOWS
#  include <foundation/windows.h>
#define IN
#define OUT
#define FAR
#define NEAR
#  include <dbghelp.h>
#  include <TlHelp32.h>
#  include <psapi.h>
#undef IN
#undef OUT
#undef FAR
#undef NEAR
#  include <stdio.h>
#elif FOUNDATION_PLATFORM_POSIX
#  include <foundation/posix.h>
#  if !FOUNDATION_PLATFORM_ANDROID
#    include <execinfo.h>
#  endif
#endif

#if FOUNDATION_PLATFORM_ANDROID
#  include <unwind.h>
#endif

#if FOUNDATION_PLATFORM_LINUX_RASPBERRYPI
extern void FOUNDATION_NOINLINE _gcc_barrier_function( uint32_t fp );
void __attribute__((optimize("O0"))) _gcc_barrier_function( uint32_t fp ) { FOUNDATION_UNUSED( fp ); }
#endif

#if FOUNDATION_PLATFORM_WINDOWS

#  if FOUNDATION_COMPILER_MSVC
#    pragma optimize( "", off )
#  elif FOUNDATION_COMPILER_CLANG
#    undef WINAPI
#    define WINAPI STDCALL
#  endif

typedef BOOL    (WINAPI *EnumProcessesFn)( DWORD* lpidProcess, DWORD cb, DWORD* cbNeeded );
typedef BOOL    (WINAPI *EnumProcessModulesFn)( HANDLE hProcess, HMODULE* lphModule, DWORD cb, LPDWORD lpcbNeeded );
typedef DWORD   (WINAPI *GetModuleBaseNameFn)( HANDLE hProcess, HMODULE hModule, LPSTR lpBaseName, DWORD nSize );
typedef DWORD   (WINAPI *GetModuleFileNameExFn)( HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize );
typedef BOOL    (WINAPI *GetModuleInformationFn)( HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb );

typedef BOOL    (WINAPI *SymInitializeFn)( HANDLE hProcess, PCSTR UserSearchPath, BOOL fInvadeProcess );
typedef DWORD   (WINAPI *SymSetOptionsFn)( DWORD SymOptions );
typedef DWORD   (WINAPI *SymGetOptionsFn)( VOID );
typedef DWORD64 (WINAPI *SymLoadModule64Fn)( HANDLE hProcess, HANDLE hFile, PCSTR ImageName, PCSTR ModuleName, DWORD64 BaseOfDll, DWORD SizeOfDll );
typedef BOOL    (WINAPI *SymSetSearchPathFn)( HANDLE hProcess, PCSTR SearchPath );
typedef BOOL    (WINAPI *SymGetModuleInfo64Fn)( HANDLE hProcess, DWORD64 qwAddr, PIMAGEHLP_MODULE64 ModuleInfo );
typedef BOOL    (WINAPI *SymGetLineFromAddr64Fn)( HANDLE hProcess, DWORD64 qwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line64 );
typedef BOOL    (WINAPI *SymGetSymFromAddr64Fn)( HANDLE hProcess, DWORD64 qwAddr, PDWORD64 pdwDisplacement, PIMAGEHLP_SYMBOL64 Symbol );
typedef DWORD64 (WINAPI *SymGetModuleBase64Fn)( HANDLE hProcess, DWORD64 qwAddr );
typedef PVOID   (WINAPI *SymFunctionTableAccess64Fn)( HANDLE hProcess, DWORD64 AddrBase );

typedef BOOL    (WINAPI* StackWalk64Fn)( DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress );
typedef WORD    (WINAPI* RtlCaptureStackBackTraceFn)( DWORD FramesToSkip, DWORD FramesToCapture, PVOID *BackTrace, PDWORD BackTraceHash );

static EnumProcessesFn         CallEnumProcesses;
static EnumProcessModulesFn    CallEnumProcessModules;
static GetModuleBaseNameFn     CallGetModuleBaseName;
static GetModuleFileNameExFn   CallGetModuleFileNameEx;
static GetModuleInformationFn  CallGetModuleInformation;

static SymInitializeFn             CallSymInitialize;
static SymSetOptionsFn             CallSymSetOptions;
static SymGetOptionsFn             CallSymGetOptions;
static SymLoadModule64Fn           CallSymLoadModule64;
static SymSetSearchPathFn          CallSymSetSearchPath;
static SymGetModuleInfo64Fn        CallSymGetModuleInfo64;
static SymGetLineFromAddr64Fn      CallSymGetLineFromAddr64;
static SymGetSymFromAddr64Fn       CallSymGetSymFromAddr64;
static SymGetModuleBase64Fn        CallSymGetModuleBase64;
static SymFunctionTableAccess64Fn  CallSymFunctionTableAccess64;

static StackWalk64Fn               CallStackWalk64;
static RtlCaptureStackBackTraceFn  CallRtlCaptureStackBackTrace;

#  define USE_CAPTURESTACKBACKTRACE 1

#  if FOUNDATION_COMPILER_GCC || FOUNDATION_COMPILER_CLANG

LONG WINAPI _stacktrace_exception_filter( LPEXCEPTION_POINTERS pointers )
{
	log_errorf( 0, ERROR_EXCEPTION, "Exception occurred in stack trace!" );
	return EXCEPTION_EXECUTE_HANDLER;
}

#  endif

static int _capture_stack_trace_helper( void** trace, unsigned int max_depth, unsigned int skip_frames, CONTEXT* context )
{
	STACKFRAME64   stack_frame;
	HANDLE         process_handle;
	HANDLE         thread_handle;
	unsigned long  last_error;
	bool           succeeded = true;
	unsigned int   current_depth = 0;
	unsigned int   machine_type	= IMAGE_FILE_MACHINE_I386;
	CONTEXT        context_copy = *context;

#if FOUNDATION_COMPILER_GCC || FOUNDATION_COMPILER_CLANG
	LPTOP_LEVEL_EXCEPTION_FILTER prev_filter = SetUnhandledExceptionFilter( _stacktrace_exception_filter );
#else
	__try
#endif
	{
		process_handle = GetCurrentProcess();
		thread_handle  = GetCurrentThread();

		memset( &stack_frame, 0, sizeof( stack_frame ) );

		stack_frame.AddrPC.Mode         = AddrModeFlat;
		stack_frame.AddrStack.Mode      = AddrModeFlat;
		stack_frame.AddrFrame.Mode      = AddrModeFlat;
#if FOUNDATION_ARCH_X86_64
		stack_frame.AddrPC.Offset       = context->Rip;
		stack_frame.AddrStack.Offset    = context->Rsp;
		stack_frame.AddrFrame.Offset    = context->Rbp;
		machine_type                    = IMAGE_FILE_MACHINE_AMD64;
#else
		stack_frame.AddrPC.Offset       = context->Eip;
		stack_frame.AddrStack.Offset    = context->Esp;
		stack_frame.AddrFrame.Offset    = context->Ebp;
#endif

		while( succeeded && ( current_depth < max_depth ) )
		{
			succeeded = CallStackWalk64( machine_type, process_handle, thread_handle, &stack_frame, &context_copy, 0, CallSymFunctionTableAccess64, CallSymGetModuleBase64, 0 );
			if( !succeeded )
				last_error = GetLastError();
			else if( !stack_frame.AddrFrame.Offset || !stack_frame.AddrPC.Offset )
				break;
			else if( skip_frames )
				--skip_frames;
			else
				trace[current_depth++] = (void*)((uintptr_t)stack_frame.AddrPC.Offset);
		}
	}
#if FOUNDATION_COMPILER_GCC || FOUNDATION_COMPILER_CLANG
	SetUnhandledExceptionFilter( prev_filter );
#else
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		// We need to catch any exceptions within this function so they don't get sent to
		// the engine's error handler, hence causing an infinite loop.
		return EXCEPTION_EXECUTE_HANDLER;
	}
#endif

	memset( trace + current_depth, 0, sizeof( void* ) * ( max_depth - current_depth ) );

	return EXCEPTION_EXECUTE_HANDLER;
}

#define MAX_MOD_HANDLES   1024

static void _load_process_modules()
{
	int           error = 0;
	bool          succeeded;
	HMODULE       module_handles[MAX_MOD_HANDLES];
	HMODULE*      module_handle = module_handles;
	int           module_count = 0;
	int           i;
	DWORD         bytes = 0;
	MODULEINFO    module_info;
	HANDLE        process_handle = GetCurrentProcess();

	succeeded = CallEnumProcessModules( process_handle, module_handles, sizeof( module_handles ), &bytes );
	if( !succeeded )
	{
		error = GetLastError();
		return;
	}

	if( bytes > sizeof( module_handles ) )
	{
		module_handle = memory_allocate( 0, bytes, 0, MEMORY_TEMPORARY );
		CallEnumProcessModules( process_handle, module_handle, bytes, &bytes );
	}

	module_count = bytes / sizeof( HMODULE );

	for( i = 0; i < module_count; ++i )
	{
		char module_name[1024];
		char image_name[1024];
		char search_path[1024];
		char* file_name = 0;
		uint64_t base_address;

		CallGetModuleInformation( process_handle, module_handle[i], &module_info, sizeof( module_info ) );
		CallGetModuleFileNameEx( process_handle, module_handle[i], image_name, 1024 );
		CallGetModuleBaseName( process_handle, module_handle[i], module_name, 1024 );

		GetFullPathNameA( image_name, 1024, search_path, &file_name );
		*file_name = 0;
		CallSymSetSearchPath( process_handle, search_path );

		base_address = CallSymLoadModule64( process_handle, module_handle[i], image_name, module_name, (uint64_t)((uintptr_t)module_info.lpBaseOfDll), module_info.SizeOfImage );
		if( !base_address )
		{
			error = GetLastError();
		}
	}

	// Free the module handle pointer allocated in case the static array was insufficient.
	if( module_handle != module_handles )
		memory_deallocate( module_handle );
}

#endif

#if FOUNDATION_PLATFORM_ANDROID

#define FOUNDATION_MAX_MODULES 256

struct arm_trace_t
{
	void**        trace;
	unsigned int  cur_depth;
	unsigned int  max_depth;
	unsigned int  skip_frames;
};
typedef struct arm_trace_t arm_trace_t;

#if FOUNDATION_PLATFORM_ANDROID

struct arm_module_t
{
	uintptr_t           address_start;
	uintptr_t           address_end;
	char                name[64];
};
typedef struct arm_module_t arm_module_t;

arm_module_t _process_modules[FOUNDATION_MAX_MODULES];

static void _load_process_modules()
{
	int imod = 0;
	char line_buffer[256];

	memset( _process_modules, 0, sizeof( arm_module_t ) * FOUNDATION_MAX_MODULES );

	stream_t* maps = fs_open_file( "/proc/self/maps", STREAM_IN );
	if( !maps )
	{
		log_errorf( 0, ERROR_SYSTEM_CALL_FAIL, "Unable to read /proc/self/maps" );
		return;
	}

	while( !stream_eos( maps ) && ( imod < FOUNDATION_MAX_MODULES ) )
	{
		line_buffer[0] = 0;
		stream_read_line_buffer( maps, line_buffer, 256, '\n' );

		if( !line_buffer[0] )
			continue;

		uintptr_t start = (uintptr_t)string_to_uint64( line_buffer, true );
		uintptr_t end = (uintptr_t)string_to_uint64( line_buffer + string_find( line_buffer, '-', 0 ) + 1, true );
		const char* module = line_buffer + string_rfind_first_of( line_buffer, "/ ", STRING_NPOS ) + 1;

		if( !module[0] || ( string_find_first_not_of( module, "0123456789", 0 ) == STRING_NPOS ) )
			continue;

		if( ( imod > 0 ) && /*( start == _process_modules[imod].address_end ) && */string_equal( module, _process_modules[imod-1].name ) )
		{
			_process_modules[imod-1].address_end = end;
			continue;
		}

		_process_modules[imod].address_start = start;
		_process_modules[imod].address_end = end;
		string_copy( _process_modules[imod].name, module, 64 );

		++imod;
	}

	//for( int i = 0; i < imod; ++i )
	//	log_infof( HASH_TEST, "%" PRIfixPTR "-%" PRIfixPTR ": %s", _process_modules[i].address_start, _process_modules[i].address_end, _process_modules[i].name );

	if( imod == FOUNDATION_MAX_MODULES )
		log_warnf( 0, WARNING_MEMORY, "Too many modules encountered" );

	stream_deallocate( maps );
}

#endif

#if FOUNDATION_COMPILER_CLANG && FOUNDATION_ARCH_ARM && !FOUNDATION_ARCH_ARM_64

extern int _Unwind_VRS_Get(struct _Unwind_Context *context, int regclass, uint32_t regno, int representation, void* valuep);

uintptr_t _Unwind_GetGR( struct _Unwind_Context* ctx, int index )
{
	#define _UVRSC_CORE 0    // integer register
	#define _UVRSD_UINT32 0
	uint32_t val;
	_Unwind_VRS_Get( ctx, _UVRSC_CORE, index, _UVRSD_UINT32, &val );
	return val;
}

uintptr_t _Unwind_GetIP( struct _Unwind_Context* ctx )
{
	#define UNWIND_IP_REG 15
	return _Unwind_GetGR( ctx, UNWIND_IP_REG ) & ~1; // thumb bit
}

#endif

static _Unwind_Reason_Code unwind_stack( struct _Unwind_Context* context, void* arg )
{
	arm_trace_t* trace = arg;
	void* ip = (void*)_Unwind_GetIP( context );
	if( trace->skip_frames )
		--trace->skip_frames;
	else if( ip )
	{
		if( trace->cur_depth < trace->max_depth )
			trace->trace[trace->cur_depth++] = ip;
		else
			return 4;//_URC_NORMAL_STOP;
	}
	return _URC_NO_REASON;
}

#endif


static bool _stackwalk_initialized = false;

static bool _initialize_stackwalker()
{
	if( _stackwalk_initialized )
		return true;

#if FOUNDATION_PLATFORM_WINDOWS
	{
		void* dll = LoadLibraryA( "DBGHELP.DLL" );
		CallStackWalk64 = dll ? (StackWalk64Fn)GetProcAddress( dll, "StackWalk64" ) : 0;
		if( !CallStackWalk64 )
		{
			log_warn( 0, WARNING_SYSTEM_CALL_FAIL, "Unable to load dbghelp DLL for StackWalk64" );
			return false;
		}

		dll = LoadLibraryA( "NTDLL.DLL" );
		CallRtlCaptureStackBackTrace = dll ? (RtlCaptureStackBackTraceFn)GetProcAddress( dll, "RtlCaptureStackBackTrace" ) : 0;
		if( !CallRtlCaptureStackBackTrace )
		{
			log_warn( 0, WARNING_SYSTEM_CALL_FAIL, "Unable to load ntdll DLL for RtlCaptureStackBackTrace" );
			return false;
		}
	}
#endif

	_stackwalk_initialized = true;
	return true;
}


unsigned int FOUNDATION_NOINLINE stacktrace_capture( void** trace, unsigned int max_depth, unsigned int skip_frames )
{
	unsigned int num_frames = 0;

	if( !trace )
		return 0;

	if( !max_depth )
		max_depth = BUILD_SIZE_STACKTRACE_DEPTH;

	if( max_depth > BUILD_SIZE_STACKTRACE_DEPTH )
		max_depth = BUILD_SIZE_STACKTRACE_DEPTH;

	if( !_stackwalk_initialized )
	{
		if( !_initialize_stackwalker() )
		{
			memset( trace, 0, sizeof( void* ) * max_depth );
			return num_frames;
		}
	}

#if FOUNDATION_PLATFORM_WINDOWS && ( FOUNDATION_COMPILER_MSVC || FOUNDATION_COMPILER_INTEL )
	// Add 1 skip frame for this function call
	++skip_frames;
#  if USE_CAPTURESTACKBACKTRACE
	if( CallRtlCaptureStackBackTrace )
	{
		void* local_trace[BUILD_SIZE_STACKTRACE_DEPTH];
		if( max_depth + skip_frames > BUILD_SIZE_STACKTRACE_DEPTH )
			max_depth = BUILD_SIZE_STACKTRACE_DEPTH - skip_frames;
		num_frames = (unsigned int)CallRtlCaptureStackBackTrace( skip_frames, max_depth, local_trace, 0 );
		if( num_frames > max_depth )
			num_frames = max_depth;
		memcpy( trace, local_trace, sizeof( void* ) * num_frames );
		memset( trace + num_frames, 0, sizeof( void* ) * ( max_depth - num_frames ) );
	}
	else
	{
#  else
	{
#  endif
#  if FOUNDATION_ARCH_X86_64
	// Raise an exception so helper has access to context record.
	__try
	{
		RaiseException(	0,			// Application-defined exception code.
						0,			// Zero indicates continuable exception.
						0,			// Number of arguments in args array (ignored if args is null)
						0 );		// Array of arguments
	}
	__except( _capture_stack_trace_helper( trace, max_depth, skip_frames, (GetExceptionInformation())->ContextRecord ) )
	{
	}
#  else
	// Use a bit of inline assembly to capture the information relevant to stack walking which is
	// basically EIP and EBP.
	CONTEXT context;
	memset( &context, 0, sizeof( CONTEXT ) );
	context.ContextFlags = CONTEXT_FULL;

	log_warnf( 0, WARNING_DEPRECATED, "********** REIMPLEMENT FALLBACK STACKTRACE **********" );
	/* Use a fake function call to pop the return address and retrieve EIP.*/
	__asm
	{
		call FakeStackTraceCall
		FakeStackTraceCall:
		pop eax
		mov context.Eip, eax
		mov context.Ebp, ebp
		mov context.Esp, esp
	}

	// Capture the back trace.
	_capture_stack_trace_helper( trace, max_depth, skip_frames, &context );
#  endif
	}

#elif FOUNDATION_PLATFORM_ANDROID

	arm_trace_t stack_trace = {
		.trace = trace,
		.cur_depth = 0,
		.max_depth = max_depth,
		.skip_frames = skip_frames
	};

	_Unwind_Backtrace( unwind_stack, &stack_trace );

	num_frames = stack_trace.cur_depth;

#elif FOUNDATION_PLATFORM_LINUX_RASPBERRYPI

# define READ_32BIT_MEMORY( addr ) (*(uint32_t volatile* volatile)(addr))
	volatile uint32_t fp = 0;
	volatile uint32_t pc = 0;

	//Grab initial frame pointer
	__asm volatile("mov %[result], fp\n\t" : [result] "=r" (fp));

	_gcc_barrier_function( fp );

	while( fp && ( num_frames < max_depth ) )
	{
		pc = READ_32BIT_MEMORY( fp );
		fp = READ_32BIT_MEMORY( fp - 4 );

		if( ( fp > 0x1000 ) && pc )
		{
			if( skip_frames > 0 )
			{
				--skip_frames;
			}
			else
			{
				void* instruction = (void*)(uintptr_t)( pc & ~3 );
				trace[num_frames++] = instruction;
			}
		}
		else
		{
			fp = 0;
		}
	}

#elif FOUNDATION_PLATFORM_POSIX

	// Add 1 skip frames for this function call
	skip_frames += 1;

	void* localframes[BUILD_SIZE_STACKTRACE_DEPTH];
	num_frames = (unsigned int)backtrace( localframes, BUILD_SIZE_STACKTRACE_DEPTH );

	if( num_frames > skip_frames )
	{
		num_frames -= skip_frames;
		if( num_frames > max_depth )
			num_frames = max_depth;
		memcpy( trace, localframes + skip_frames, sizeof( void* ) * num_frames );
	}
	else
		trace[0] = 0;

#else
	FOUNDATION_UNUSED( skip_frames );
#endif

	return num_frames;
}


static bool _symbol_resolve_initialized = false;

static bool _initialize_symbol_resolve()
{
	if( _symbol_resolve_initialized )
		return true;

#if FOUNDATION_PLATFORM_WINDOWS
	{
		unsigned int options;
		void* dll = LoadLibraryA( "PSAPI.DLL" );
		if( !dll )
			return _symbol_resolve_initialized;

		CallEnumProcesses = (EnumProcessesFn)GetProcAddress( dll, "EnumProcesses" );
		CallEnumProcessModules = (EnumProcessModulesFn)GetProcAddress(  dll, "EnumProcessModules" );
		CallGetModuleFileNameEx = (GetModuleFileNameExFn)GetProcAddress(  dll, "GetModuleFileNameExA" );
		CallGetModuleBaseName = (GetModuleBaseNameFn)GetProcAddress(  dll, "GetModuleBaseNameA" );
		CallGetModuleInformation = (GetModuleInformationFn)GetProcAddress( dll, "GetModuleInformation" );

		if( !CallEnumProcesses || !CallEnumProcessModules || !CallGetModuleFileNameEx || !CallGetModuleBaseName || !CallGetModuleInformation )
			return _symbol_resolve_initialized;

		dll = LoadLibraryA( "DBGHELP.DLL" );
		if( !dll )
			return _symbol_resolve_initialized;

		CallSymInitialize = (SymInitializeFn)GetProcAddress( dll, "SymInitialize" );
		CallSymSetOptions = (SymSetOptionsFn)GetProcAddress( dll, "SymSetOptions" );
		CallSymGetOptions = (SymGetOptionsFn)GetProcAddress( dll, "SymGetOptions" );
		CallSymLoadModule64 = (SymLoadModule64Fn)GetProcAddress( dll, "SymLoadModule64" );
		CallSymSetSearchPath = (SymSetSearchPathFn)GetProcAddress( dll, "SymSetSearchPath" );
		CallSymGetModuleInfo64 = (SymGetModuleInfo64Fn)GetProcAddress( dll, "SymGetModuleInfo64" );
		CallSymGetLineFromAddr64 = (SymGetLineFromAddr64Fn)GetProcAddress( dll, "SymGetLineFromAddr64" );
		CallSymGetSymFromAddr64 = (SymGetSymFromAddr64Fn)GetProcAddress( dll, "SymGetSymFromAddr64" );
		CallSymGetModuleBase64 = (SymGetModuleBase64Fn)GetProcAddress( dll, "SymGetModuleBase64" );
		CallSymFunctionTableAccess64 = (SymFunctionTableAccess64Fn)GetProcAddress( dll, "SymFunctionTableAccess64" );

		if( !CallSymInitialize || !CallSymSetOptions || !CallSymGetOptions || !CallSymLoadModule64 || !CallSymSetSearchPath || !CallSymGetModuleInfo64 || !CallSymGetLineFromAddr64 || !CallSymGetSymFromAddr64  || !CallSymGetModuleBase64 || !CallSymFunctionTableAccess64 )
			return _symbol_resolve_initialized;

		options = CallSymGetOptions();
		options |= SYMOPT_LOAD_LINES;
		options |= SYMOPT_DEBUG;
		options |= SYMOPT_UNDNAME;
		options |= SYMOPT_LOAD_LINES;
		options |= SYMOPT_FAIL_CRITICAL_ERRORS;
		options |= SYMOPT_DEFERRED_LOADS;
		options |= SYMOPT_ALLOW_ABSOLUTE_SYMBOLS;
		options |= SYMOPT_EXACT_SYMBOLS;
		options |= SYMOPT_CASE_INSENSITIVE;
		CallSymSetOptions( options );

		CallSymInitialize( GetCurrentProcess(), 0, TRUE );
	}

	_load_process_modules();

	_symbol_resolve_initialized = true;

#else

	_symbol_resolve_initialized = true;

#endif

	return _symbol_resolve_initialized;
}


static FOUNDATION_NOINLINE char** _resolve_stack_frames( void** frames, unsigned int max_frames )
{
#if FOUNDATION_PLATFORM_WINDOWS
	char**              lines = 0;
	char                symbol_buffer[ sizeof( IMAGEHLP_SYMBOL64 ) + 512 ];
	PIMAGEHLP_SYMBOL64  symbol;
	DWORD               displacement = 0;
	uint64_t            displacement64 = 0;
	unsigned int        iaddr = 0;
	unsigned int        last_error;
	bool                found = false;
	HANDLE              process_handle = GetCurrentProcess();
	int                 buffer_offset = 0;
	bool                last_was_main = false;
	IMAGEHLP_LINE64     line64;
	IMAGEHLP_MODULE64   module64;

	for( iaddr = 0; ( iaddr < max_frames ) && !last_was_main; ++iaddr )
	{
		char* resolved = 0;
		const char* function_name = "??";
		const char* file_name = "??";
		const char* module_name = "??";
		unsigned int line_number = 0;

		//Allow first frame to be null in case of a function call to a null pointer
		if( iaddr && !frames[iaddr] )
			break;

		// Initialize symbol.
		symbol = (PIMAGEHLP_SYMBOL64)symbol_buffer;
		memset( symbol, 0, sizeof( symbol_buffer ) );
		symbol->SizeOfStruct = sizeof( symbol_buffer );
		symbol->MaxNameLength = 512;

		// Get symbol from address.
		if( CallSymGetSymFromAddr64 && CallSymGetSymFromAddr64( process_handle, (uint64_t)((uintptr_t)frames[iaddr]), &displacement64, symbol ) )
		{
			int offset = 0;
			while( symbol->Name[offset] < 32 )
				++offset;
			function_name = symbol->Name + offset;
		}
		else
		{
			// No symbol found for this address.
			last_error = GetLastError();
		}

		memset( &line64, 0, sizeof( line64 ) );
		line64.SizeOfStruct = sizeof( line64 );
		if( CallSymGetLineFromAddr64 && CallSymGetLineFromAddr64( process_handle, (uint64_t)((uintptr_t)frames[iaddr]), &displacement, &line64 ) )
		{
			file_name = line64.FileName;
			line_number = line64.LineNumber;
		}

		memset( &module64, 0, sizeof( module64 ) );
		module64.SizeOfStruct = sizeof( module64 );
		if( CallSymGetModuleInfo64 && CallSymGetModuleInfo64( process_handle, (uint64_t)((uintptr_t)frames[iaddr]), &module64 ) )
		{
			int last_slash = STRING_NPOS;
			module_name = module64.ImageName;
			last_slash = string_rfind( module_name, '\\', STRING_NPOS );
			if( last_slash != STRING_NPOS )
				module_name += last_slash + 1;
		}

		resolved = string_format( "[0x%" PRIfixPTR "] %s (%s:%d +%d bytes) [in %s]", frames[iaddr], function_name, file_name, line_number, displacement, module_name );
		array_push( lines, resolved );

		if( string_equal( function_name, "main" ) )
			last_was_main = true;
	}

	return lines;

#elif FOUNDATION_PLATFORM_MACOSX || FOUNDATION_PLATFORM_IOS

	char** symbols = 0;
	char** resolved = backtrace_symbols( frames, max_frames );
	for( unsigned int iframe = 0; iframe < max_frames; ++iframe )
	{
		if( resolved[iframe] && string_length( resolved[iframe] ) )
			array_push( symbols, string_clone( resolved[iframe] ) );
	}

	return symbols;

#elif FOUNDATION_PLATFORM_LINUX || FOUNDATION_PLATFORM_BSD

	char** addrs = 0;
	char** lines = 0;
	const char** args = 0;
	process_t* proc = process_allocate();
	unsigned int num_frames = 0;
	unsigned int requested_frames = 0;
	bool last_was_main = false;

	if( !string_length( environment_executable_path() ) )
	{
		for( unsigned int iaddr = 0; iaddr < max_frames; ++iaddr )
		{
			//Allow first frame to be null in case of a function call to a null pointer
			if( iaddr && !frames[iaddr] )
				break;

			array_push( lines, string_format( "[0x%" PRIfixPTR "]", frames[iaddr] ) );
		}
		return lines;
	}

	array_push( args, "-e" );
	array_push( args, environment_executable_path() );
	array_push( args, "-f" );

	for( unsigned int iaddr = 0; iaddr < max_frames; ++iaddr )
	{
		//Allow first frame to be null in case of a function call to a null pointer
		if( iaddr && !frames[iaddr] )
			break;

		char* addr = string_format( "0x%" PRIfixPTR, frames[iaddr] );
		array_push( addrs, addr );
		array_push( args, addr );

		++requested_frames;
	}

	process_set_working_directory( proc, environment_initial_working_directory() );
	process_set_executable_path( proc, "/usr/bin/addr2line" );
	process_set_arguments( proc, args, array_size( args ) );
	process_set_flags( proc, PROCESS_ATTACHED | PROCESS_STDSTREAMS );

	process_spawn( proc );

	stream_t* procout = process_stdout( proc );
	while( !stream_eos( procout ) && ( num_frames < requested_frames ) && !last_was_main )
	{
		char* function = stream_read_line( procout, '\n' );
		char* filename = stream_read_line( procout, '\n' );

		array_push( lines, string_format( "[0x%" PRIfixPTR "] %s (%s)",
			frames[num_frames],
			function && string_length( function ) ? function : "??",
			filename && string_length( filename ) ? filename : "??"
		) );

		if( string_equal( function, "main" ) )
			last_was_main = true;

		string_deallocate( function );
		string_deallocate( filename );

		++num_frames;
	}

	process_wait( proc );
	process_deallocate( proc );

	string_array_deallocate( addrs );
	array_deallocate( args );

	return lines;

#elif FOUNDATION_PLATFORM_ANDROID

	char** lines = 0;

	_load_process_modules();

	for( unsigned int iaddr = 0; iaddr < max_frames; ++iaddr )
	{
		//Allow first frame to be null in case of a function call to a null pointer
		if( iaddr && !frames[iaddr] )
			break;

		//Find the module and relative address
		uintptr_t relativeframe = (uintptr_t)frames[iaddr];
		const char* module = "<no module found>";

		for( int imod = 0; imod < FOUNDATION_MAX_MODULES; ++imod )
		{
			if( ( relativeframe >= _process_modules[imod].address_start ) && ( relativeframe < _process_modules[imod].address_end ) )
			{
				relativeframe -= _process_modules[imod].address_start;
				module = _process_modules[imod].name;
				break;
			}
		}

		array_push( lines, string_format( "[0x%" PRIfixPTR "] 0x%" PRIfixPTR " %s", frames[iaddr], relativeframe, module ) );
	}

	return lines;

#else

	char** lines = 0;
	for( unsigned int iaddr = 0; iaddr < max_frames; ++iaddr )
	{
		//Allow first frame to be null in case of a function call to a null pointer
		if( iaddr && !frames[iaddr] )
			break;

		array_push( lines, string_format( "[0x%" PRIfixPTR "]", frames[iaddr] ) );
	}

	return lines;

#endif
}


char* stacktrace_resolve( void** trace, unsigned int max_depth, unsigned int skip_frames )
{
	char** lines;
	char* resolved;

	_initialize_symbol_resolve();

	if( !max_depth )
		max_depth = BUILD_SIZE_STACKTRACE_DEPTH;
	if( max_depth + skip_frames > BUILD_SIZE_STACKTRACE_DEPTH )
		max_depth = BUILD_SIZE_STACKTRACE_DEPTH - skip_frames;

	lines = _resolve_stack_frames( trace + skip_frames, max_depth );

	resolved = string_merge( (const char* const*)lines, array_size( lines ), "\n" );

	string_array_deallocate( lines );

	return resolved;
}
