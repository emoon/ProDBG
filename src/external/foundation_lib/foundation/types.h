/* types.h  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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
#include <foundation/build.h>

#if defined( FOUNDATION_PLATFORM_DOXYGEN )
#  define FOUNDATION_ALIGNED_STRUCT( name, alignment ) struct name
#endif

#if FOUNDATION_COMPILER_CLANG
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wpadded"
#endif


// PRIMITIVE TYPES

typedef enum
{
	ERRORLEVEL_NONE    = 0,
	ERRORLEVEL_DEBUG,
	ERRORLEVEL_INFO,
	ERRORLEVEL_WARNING,
	ERRORLEVEL_ERROR,
	ERRORLEVEL_PANIC
} error_level_t;

typedef enum
{
	ERROR_NONE              = 0,
	ERROR_INVALID_VALUE,
	ERROR_UNSUPPORTED,
	ERROR_NOT_IMPLEMENTED,
	ERROR_OUT_OF_MEMORY,
	ERROR_MEMORY_LEAK,
	ERROR_MEMORY_ALIGNMENT,
	ERROR_INTERNAL_FAILURE,
	ERROR_ACCESS_DENIED,
	ERROR_EXCEPTION,
	ERROR_SYSTEM_CALL_FAIL,
	ERROR_UNKNOWN_TYPE,
	ERROR_UNKNOWN_RESOURCE,
	ERROR_DEPRECATED,
	ERROR_ASSERT,
	ERROR_SCRIPT,
	ERROR_LAST_BUILTIN  = 0x0fff
} error_t;

typedef enum
{
	WARNING_PERFORMANCE = 0,
	WARNING_DEPRECATED,
	WARNING_BAD_DATA,
	WARNING_MEMORY,
	WARNING_UNSUPPORTED,
	WARNING_SUSPICIOUS,
	WARNING_SYSTEM_CALL_FAIL,
	WARNING_DEADLOCK,
	WARNING_SCRIPT,
	WARNING_RESOURCE,
	WARNING_LAST_BUILTIN  = 0x0fff
} warning_t;

typedef enum
{
	MEMORY_PERSISTENT          = 0x0000,
	MEMORY_TEMPORARY           = 0x0001,
	MEMORY_THREAD              = 0x0002,
	MEMORY_32BIT_ADDRESS       = 0x0004,
	MEMORY_ZERO_INITIALIZED    = 0x0008
} memory_hint_t;

typedef enum
{
	PLATFORM_WINDOWS   = 0,
	PLATFORM_LINUX,
	PLATFORM_MACOSX,
	PLATFORM_IOS,
	PLATFORM_ANDROID,
	PLATFORM_RASPBERRYPI,
	PLATFORM_PNACL,
	PLATFORM_BSD,
	PLATFORM_TIZEN,

	PLATFORM_INVALID
} platform_t;

typedef enum
{
	ARCHITECTURE_X86          = 0,
	ARCHITECTURE_X86_64,
	ARCHITECTURE_PPC,
	ARCHITECTURE_PPC_64,
	ARCHITECTURE_ARM5,
	ARCHITECTURE_ARM6,
	ARCHITECTURE_ARM7,
	ARCHITECTURE_ARM8,
	ARCHITECTURE_ARM8_64,
	ARCHITECTURE_MIPS,
	ARCHITECTURE_MIPS_64,
	ARCHITECTURE_GENERIC
} architecture_t;

typedef enum
{
	BYTEORDER_LITTLEENDIAN = 0,
	BYTEORDER_BIGENDIAN    = 1
} byteorder_t;

typedef enum
{
	APPLICATION_UTILITY        = 0x0001,
	APPLICATION_DAEMON         = 0x0002
} application_flag_t;

typedef enum
{
	STREAM_IN                  = 0x0001,
	STREAM_OUT                 = 0x0002,
	STREAM_TRUNCATE            = 0x0010,
	STREAM_CREATE              = 0x0020,
	STREAM_ATEND               = 0x0040,
	STREAM_BINARY              = 0x0100,
	STREAM_SYNC                = 0x0200
} stream_mode_t;

typedef enum
{
	STREAMTYPE_INVALID         = 0,
	STREAMTYPE_MEMORY,
	STREAMTYPE_FILE,
	STREAMTYPE_SOCKET,
	STREAMTYPE_RINGBUFFER,
	STREAMTYPE_ASSET,
	STREAMTYPE_PIPE,
	STREAMTYPE_STDSTREAM,
	STREAMTYPE_LAST_RESERVED   = 0x0FFF
} stream_type_t;

typedef enum
{
	FL_STREAM_SEEK_BEGIN          = 0x0000,
	FL_STREAM_SEEK_CURRENT        = 0x0001,
	FL_STREAM_SEEK_END            = 0x0002
} stream_seek_mode_t;

typedef enum
{
	THREAD_PRIORITY_LOW = 0,
	THREAD_PRIORITY_BELOWNORMAL,
	THREAD_PRIORITY_NORMAL,
	THREAD_PRIORITY_ABOVENORMAL,
	THREAD_PRIORITY_HIGHEST,
	THREAD_PRIORITY_TIMECRITICAL
} thread_priority_t;

typedef enum
{
	PROCESS_ATTACHED                          = 0,
	PROCESS_DETACHED                          = 0x01,
	PROCESS_CONSOLE                           = 0x02,
	PROCESS_STDSTREAMS                        = 0x04,
	PROCESS_WINDOWS_USE_SHELLEXECUTE          = 0x08,
	PROCESS_MACOSX_USE_OPENAPPLICATION        = 0x10
} process_flag_t;

typedef enum
{
	PROCESS_INVALID_ARGS                      = 0x7FFFFFF0,
	PROCESS_TERMINATED_SIGNAL                 = 0x7FFFFFF1,
	PROCESS_WAIT_INTERRUPTED                  = 0x7FFFFFF2,
	PROCESS_WAIT_FAILED                       = 0x7FFFFFF3,
	PROCESS_STILL_ACTIVE                      = 0x7FFFFFFF
} process_status_t;

typedef enum
{
	FOUNDATIONEVENT_START = 1,
	FOUNDATIONEVENT_TERMINATE,
	FOUNDATIONEVENT_PAUSE,
	FOUNDATIONEVENT_RESUME,
	FOUNDATIONEVENT_FOCUS_GAIN,
	FOUNDATIONEVENT_FOCUS_LOST,
	FOUNDATIONEVENT_FILE_CREATED,
	FOUNDATIONEVENT_FILE_DELETED,
	FOUNDATIONEVENT_FILE_MODIFIED,
	FOUNDATIONEVENT_LOW_MEMORY_WARNING,
	FOUNDATIONEVENT_DEVICE_ORIENTATION
} foundation_event_id;

typedef enum
{
	EVENTFLAG_DELAY  = 1
} event_flag_t;

typedef enum
{
	BLOWFISH_ECB = 0,
	BLOWFISH_CBC,
	BLOWFISH_CFB,
	BLOWFISH_OFB
} blowfish_mode_t;

typedef enum
{
	RADIXSORT_INT32 = 0,
	RADIXSORT_UINT32,
	RADIXSORT_INT64,
	RADIXSORT_UINT64,
	RADIXSORT_FLOAT32,
	RADIXSORT_FLOAT64
} radixsort_data_t;

typedef enum
{
	DEVICEORIENTATION_UNKNOWN = 0,
	DEVICEORIENTATION_PORTRAIT,
	DEVICEORIENTATION_PORTRAIT_FLIPPED,
	DEVICEORIENTATION_LANDSCAPE_CCW,
	DEVICEORIENTATION_LANDSCAPE_CW,
	DEVICEORIENTATION_FACEUP,
	DEVICEORIENTATION_FACEDOWN
} device_orientation_t;

typedef uint64_t      hash_t;
typedef uint64_t      tick_t;
typedef real          deltatime_t;
typedef uint64_t      object_t;
typedef uint16_t      radixsort_index_t;
typedef uint128_t     uuid_t;

typedef union { int32_t ival; float32_t fval; } float32_cast_t;
typedef union { int64_t ival; float64_t fval; } float64_cast_t;
#if FOUNDATION_SIZE_REAL == 64
typedef union { int64_t ival; real rval; } real_cast_t;
#else
typedef union { int32_t ival; real rval; } real_cast_t;
#endif

typedef struct application_t                 application_t;
typedef struct bitbuffer_t                   bitbuffer_t;
typedef struct blowfish_t                    blowfish_t;
typedef struct error_frame_t                 error_frame_t;
typedef struct error_context_t               error_context_t;
typedef struct event_t                       event_t;
typedef struct event_block_t                 event_block_t;
typedef struct event_stream_t                event_stream_t;
typedef struct hashmap_node_t                hashmap_node_t;
typedef struct hashmap_t                     hashmap_t;
typedef struct hashtable32_entry_t           hashtable32_entry_t;
typedef struct hashtable64_entry_t           hashtable64_entry_t;
typedef struct hashtable32_t                 hashtable32_t;
typedef struct hashtable64_t                 hashtable64_t;
typedef struct md5_t                         md5_t;
typedef struct memory_context_t              memory_context_t;
typedef struct memory_system_t               memory_system_t;
typedef struct memory_tracker_t              memory_tracker_t;
typedef struct mutex_t                       mutex_t;
typedef struct object_base_t                 object_base_t;
typedef struct objectmap_t                   objectmap_t;
typedef struct process_t                     process_t;
typedef struct radixsort_t                   radixsort_t;
typedef struct regex_t                       regex_t;
typedef struct regex_capture_t               regex_capture_t;
typedef struct ringbuffer_t                  ringbuffer_t;
typedef struct stream_t                      stream_t;
typedef struct stream_buffer_t               stream_buffer_t;
typedef struct stream_pipe_t                 stream_pipe_t;
typedef struct stream_ringbuffer_t           stream_ringbuffer_t;
typedef struct stream_vtable_t               stream_vtable_t;
typedef union  version_t                     version_t;

#if FOUNDATION_PLATFORM_WINDOWS
typedef void*                                semaphore_t;
#elif FOUNDATION_PLATFORM_MACOSX
typedef struct OpaqueMPSemaphoreID*          MPSemaphoreID;
typedef struct semaphore_t                   semaphore_t;
#elif FOUNDATION_PLATFORM_IOS
typedef struct dispatch_semaphore_s*         semaphore_t;
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
typedef union semaphore_native_t             semaphore_native_t;
typedef struct semaphore_t                   semaphore_t;
#endif

typedef int           (* error_callback_fn )( error_level_t level, error_t error );
typedef int           (* assert_handler_fn )( uint64_t context, const char* condition, const char* file, int line, const char* msg );
typedef void          (* log_callback_fn )( uint64_t context, int severity, const char* msg );
typedef int           (* system_initialize_fn )( void );
typedef void          (* system_shutdown_fn )( void );
typedef void*         (* memory_allocate_fn )( uint64_t context, uint64_t size, unsigned int align, int hint );
typedef void*         (* memory_reallocate_fn )( void* p, uint64_t size, unsigned int align, uint64_t oldsize );
typedef void          (* memory_deallocate_fn )( void* p );
typedef void          (* memory_track_fn )( void* p, uint64_t size );
typedef void          (* memory_untrack_fn )( void* p );
typedef void          (* profile_write_fn )( void* data, uint64_t size );
typedef void          (* profile_read_fn )( void* data, uint64_t size );
typedef void*         (* thread_fn )( object_t thread, void* arg );
typedef int           (* crash_guard_fn )( void* arg );
typedef void          (* crash_dump_callback_fn )( const char* file );
typedef void          (* object_deallocate_fn )( object_t id, void* object );
typedef stream_t*     (* stream_open_fn )( const char* path, unsigned int mode );
typedef uint64_t      (* stream_read_fn )( stream_t* stream, void* dst, uint64_t size );
typedef uint64_t      (* stream_write_fn )( stream_t* stream, const void* src, uint64_t size );
typedef bool          (* stream_eos_fn )( stream_t* stream );
typedef void          (* stream_flush_fn )( stream_t* stream );
typedef void          (* stream_truncate_fn  )( stream_t* stream, uint64_t size );
typedef uint64_t      (* stream_size_fn )( stream_t* stream );
typedef void          (* stream_seek_fn )( stream_t* stream, int64_t offset, stream_seek_mode_t mode );
typedef int64_t       (* stream_tell_fn )( stream_t* stream );
typedef uint64_t      (* stream_lastmod_fn )( const stream_t* stream );
typedef uint128_t     (* stream_md5_fn)( stream_t* stream );
typedef void          (* stream_buffer_read_fn )( stream_t* stream );
typedef uint64_t      (* stream_available_read_fn )( stream_t* stream );
typedef void          (* stream_finalize_fn )( stream_t* stream );
typedef stream_t*     (* stream_clone_fn )( stream_t* stream );


// Identifier returned from threads and crash guards after a fatal exception (crash) has been caught
#define FOUNDATION_CRASH_DUMP_GENERATED        0x0badf00dL


// COMPLEX TYPES

struct md5_t
{
	bool                            init;
	uint32_t                        state[4];
	uint32_t                        count[2];
	unsigned char                   buffer[64];
	unsigned char                   digest[16];
};


struct memory_system_t
{
	memory_allocate_fn              allocate;
	memory_reallocate_fn            reallocate;
	memory_deallocate_fn            deallocate;
	system_initialize_fn            initialize;
	system_shutdown_fn              shutdown;
};


struct memory_tracker_t
{
	memory_track_fn                 track;
	memory_untrack_fn               untrack;
	system_initialize_fn            initialize;
	system_shutdown_fn              shutdown;
};


union version_t
{
	uint128_t                       version;
	struct
	{
		uint16_t                    major;
		uint16_t                    minor;
		uint32_t                    revision;
		uint32_t                    build;
		uint32_t                    control;
	}                               sub;
};


struct application_t
{
	const char*                     name;
	const char*                     short_name;
	const char*                     config_dir;
	version_t                       version;
	crash_dump_callback_fn          dump_callback;
	unsigned int                    flags;
	uuid_t                          instance;
};


#define BLOWFISH_SUBKEYS            18U
#define BLOWFISH_SBOXES             4U
#define BLOWFISH_SBOXENTRIES        256U
#define BLOWFISH_MAXKEY             56U

struct blowfish_t
{
	uint32_t                        parray[BLOWFISH_SUBKEYS];
	uint32_t                        sboxes[BLOWFISH_SBOXES][BLOWFISH_SBOXENTRIES];
};


struct bitbuffer_t
{
	uint8_t*                        buffer;
	uint8_t*                        end;
	stream_t*                       stream;
	bool                            swap;
	unsigned int                    pending_read;
	unsigned int                    pending_write;
	unsigned int                    offset_read;
	unsigned int                    offset_write;
	unsigned int                    count_read;
	unsigned int                    count_write;
};


struct error_frame_t
{
	const char*                     name;
	const char*                     data;
};


struct error_context_t
{
	error_frame_t                   frame[BUILD_SIZE_ERROR_CONTEXT_DEPTH];
	int                             depth;
};


#define FOUNDATION_DECLARE_EVENT                 \
	uint16_t                        id;          \
	uint16_t                        flags;       \
	uint16_t                        serial;      \
	uint16_t                        size;        \
	object_t                        object

struct event_t
{
	FOUNDATION_DECLARE_EVENT;
	char                            payload[];
};


struct event_block_t
{
	int32_t                         used;
	uint32_t                        capacity;
	event_stream_t*                 stream;
	event_t*                        events;
};


FOUNDATION_ALIGNED_STRUCT( event_stream_t, 16 )
{
	atomic32_t                      write;
	int32_t                         read;
	event_block_t                   block[2];
};


struct hashmap_node_t
{
	hash_t                          key;
	void*                           value;
};


struct hashmap_t
{
	unsigned int                    num_buckets;
	unsigned int                    num_nodes;
	hashmap_node_t*                 bucket[];
};


FOUNDATION_ALIGNED_STRUCT( hashtable32_entry_t, 8 )
{
	atomic32_t                      key;
	uint32_t                        value;
};


FOUNDATION_ALIGNED_STRUCT( hashtable64_entry_t, 8 )
{
	atomic64_t                      key;
	uint64_t                        value;
};


FOUNDATION_ALIGNED_STRUCT( hashtable32_t, 8 )
{
	uint32_t                        capacity;
	hashtable32_entry_t             entries[];
};


FOUNDATION_ALIGNED_STRUCT( hashtable64_t, 8 )
{
	uint64_t                        capacity;
	hashtable64_entry_t             entries[];
};


struct memory_context_t
{
	uint64_t                        context[BUILD_SIZE_MEMORY_CONTEXT_DEPTH];
	int                             depth;
};

#define FOUNDATION_DECLARE_OBJECT               \
	atomic32_t                      ref;        \
	uint32_t                        flags;      \
	object_t                        id

FOUNDATION_ALIGNED_STRUCT( object_base_t, 8 )
{
	FOUNDATION_DECLARE_OBJECT;
};

FOUNDATION_ALIGNED_STRUCT( objectmap_t, 16 )
{
	atomic64_t                      free;
	uint64_t                        size;
	atomic64_t                      id;
	uint64_t                        size_bits;
	uint64_t                        id_max;
	uint64_t                        mask_index;
	uint64_t                        mask_id;
	void*                           map[];
};

struct process_t
{
	char*                           wd;
	char*                           path;
	char**                          args;
	unsigned int                    flags;
	int                             code;
	stream_t*                       pipeout;
	stream_t*                       pipein;

#if FOUNDATION_PLATFORM_WINDOWS
	char*                           verb;
	void*                           hp;
	void*                           ht;
#endif
#if FOUNDATION_PLATFORM_POSIX
	int                             pid;
#endif

#if FOUNDATION_PLATFORM_MACOSX
	int                             kq;
#endif
};

struct radixsort_t
{
	radixsort_data_t                type;
	radixsort_index_t               size;
	radixsort_index_t               lastused;
	radixsort_index_t*              indices[2];
	radixsort_index_t*              histogram;
	radixsort_index_t*              offset;
};

struct regex_t
{
	unsigned int                    num_captures;
	unsigned int                    code_length;
	unsigned int                    code_allocated;
	uint8_t                         code[];
};

struct regex_capture_t
{
	const char*                     substring;
	int                             length;
};

#define FOUNDATION_DECLARE_RINGBUFFER              \
	uint64_t                        total_read;    \
	uint64_t                        total_write;   \
	unsigned int                    offset_read;   \
	unsigned int                    offset_write;  \
	unsigned int                    buffer_size;   \
	char                            buffer[]

struct ringbuffer_t
{
	FOUNDATION_DECLARE_RINGBUFFER;
};

#if FOUNDATION_PLATFORM_MACOSX

struct semaphore_t
{
	char*                           name;
	union
	{
		MPSemaphoreID               unnamed;
		int*                        named;
	} sem;
};

#elif FOUNDATION_PLATFORM_IOS
#elif FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL

union semaphore_native_t
{
#  if FOUNDATION_PLATFORM_ANDROID
	volatile unsigned int           count;
#  elif FOUNDATION_PLATFORM_PNACL
	volatile int                    count;
	volatile int                    nwaiters;
#else
#  if FOUNDATION_ARCH_X86_64
	char                            __size[64];
#  else
	char                            __size[32];
#  endif
	long int                        __align;
#endif
};

struct semaphore_t
{
	char*                           name;
	semaphore_native_t*             sem;
	semaphore_native_t              unnamed;
};

#endif

#define FOUNDATION_DECLARE_STREAM                            \
unsigned int                    type:16;                 \
unsigned int                    sequential:1;            \
unsigned int                    reliable:1;              \
unsigned int                    inorder:1;               \
unsigned int                    swap:1;                  \
unsigned int                    byteorder:1;             \
unsigned int                    unused_streamflags:11;   \
unsigned int                    mode;                    \
char*                           path;                    \
stream_vtable_t*                vtable

FOUNDATION_ALIGNED_STRUCT( stream_t, 8 )
{
	FOUNDATION_DECLARE_STREAM;
};

FOUNDATION_ALIGNED_STRUCT( stream_buffer_t, 8 )
{
	FOUNDATION_DECLARE_STREAM;
	uint64_t                        current;
	uint64_t                        size;
	uint64_t                        capacity;
	void*                           buffer;
	bool                            own;
	bool                            grow;
};

FOUNDATION_ALIGNED_STRUCT( stream_pipe_t, 8 )
{
	FOUNDATION_DECLARE_STREAM;

	bool                            eos;

#if FOUNDATION_PLATFORM_WINDOWS
	void*                           handle_read;
	void*                           handle_write;
#endif
#if FOUNDATION_PLATFORM_POSIX || FOUNDATION_PLATFORM_PNACL
	int                             fd_read;
	int                             fd_write;
#endif
};

FOUNDATION_ALIGNED_STRUCT( stream_ringbuffer_t, 8 )
{
	FOUNDATION_DECLARE_STREAM;

	semaphore_t                     signal_read;
	semaphore_t                     signal_write;
	volatile int32_t                pending_read;
	volatile int32_t                pending_write;
	uint64_t                        total_size;

	FOUNDATION_DECLARE_RINGBUFFER;
};

struct stream_vtable_t
{
	stream_read_fn                  read;
	stream_write_fn                 write;
	stream_eos_fn                   eos;
	stream_flush_fn                 flush;
	stream_truncate_fn              truncate;
	stream_size_fn                  size;
	stream_seek_fn                  seek;
	stream_tell_fn                  tell;
	stream_lastmod_fn               lastmod;
	stream_md5_fn                   md5;
	stream_buffer_read_fn           buffer_read;
	stream_available_read_fn        available_read;
	stream_finalize_fn              finalize;
	stream_clone_fn                 clone;
};


// UTILITY FUNCTIONS

static FOUNDATION_FORCEINLINE FOUNDATION_CONSTCALL version_t      version_make( unsigned int major, unsigned int minor, unsigned int revision, unsigned int build, unsigned int control ) { version_t v; v.sub.major = (uint16_t)major; v.sub.minor = (uint16_t)minor; v.sub.revision = revision, v.sub.build = build; v.sub.control = control; return v; }


#if FOUNDATION_COMPILER_CLANG
#  pragma clang diagnostic pop
#endif
