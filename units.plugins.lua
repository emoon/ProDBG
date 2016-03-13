require "tundra.syntax.glob"
require "tundra.path"
require "tundra.util"
require "tundra.syntax.rust-cargo"

local native = require('tundra.native')

-----------------------------------------------------------------------------------------------------------------------

local function get_rs_src(dir)
	return Glob {
		Dir = dir,
		Extensions = { ".rs" },
		Recursive = true,
	}
end


-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "lldb_plugin",
    
    Env = {
        CPPPATH = { 
        	"api/include",
            "src/plugins/lldb",
        },

        CXXOPTS = { { 
            "-std=c++11", 
            "-Wno-padded",
            "-Wno-documentation",
            "-Wno-unused-parameter",
            "-Wno-missing-prototypes",
            "-Wno-unused-member-function",
            "-Wno-switch",
            "-Wno-switch-enum",
            "-Wno-c++98-compat-pedantic",
            "-Wno-missing-field-initializers"; Config = { "macosx-clang-*", "linux-*"} },
        },

        SHLIBOPTS = { 
            { "-Fsrc/plugins/lldb/Frameworks", "-rpath src/plugins/lldb/Frameworks", "-lstdc++"; Config = "macosx-clang-*" },
            { "-Fsrc/plugins/lldb/Frameworks", "-rpath src/plugins/lldb/Frameworks", "-lstdc++", "-coverage"; Config = "macosx_test-clang-*" },
        },

        CXXCOM = { "-stdlib=libc++"; Config = "macosx-clang-*" },
    },

    Sources = { 
        Glob {
            Dir = "src/plugins/lldb",
            Extensions = { ".c", ".cpp", ".m" },
        },

    },

    Frameworks = { "LLDB" },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "dbgeng_plugin",

    Env = {
        CPPPATH = {
            "api/include",
            "src/plugins/dbgeng",
        },
    },

    Sources = {
        "src/plugins/dbgeng/dbgeng_plugin.cpp"
    },

    Libs = { "dbgeng.lib" },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "sourcecode_plugin",
    
    Env = {
        CPPPATH = { 
        	"api/include", 
        	"src/native/external", 
        },
    	CXXOPTS = { { "-fPIC"; Config = "linux-gcc"; }, },
    },

    Sources = { "src/plugins/sourcecode/sourcecode_plugin.cpp" },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "callstack_plugin",
    
    Env = {
        CPPPATH = { "api/include", },
    	CXXOPTS = { { "-fPIC"; Config = "linux-gcc"; }, },
    },

    Sources = { "src/plugins/callstack/callstack_plugin.cpp" },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "disassembly_plugin",
    
    Env = {
        CPPPATH = { "api/include", },
    	CXXOPTS = { { "-fPIC"; Config = "linux-gcc"; }, },
    },

    Sources = { "src/plugins/disassembly/disassembly_plugin.cpp" },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "registers_plugin",
    
    Env = {
        CPPPATH = { "api/include", },
    	CXXOPTS = { { "-fPIC"; Config = "linux-gcc"; }, },
    },

    Sources = { "src/plugins/registers/registers_plugin.cpp" },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "locals_plugin",
    
    Env = {
        CPPPATH = { "api/include", },
    	CXXOPTS = { { "-fPIC"; Config = "linux-gcc"; }, },
    },

    Sources = { "src/plugins/locals/locals_plugin.cpp" },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "threads_plugin",
    
    Env = {
        CPPPATH = { "api/include", },
    	CXXOPTS = { { "-fPIC"; Config = "linux-gcc"; }, },
    },

    Sources = { "src/plugins/threads/threads_plugin.cpp" },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
}


-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "breakpoints_plugin",
    
    Env = {
        CPPPATH = { "api/include", },
    	CXXOPTS = { { "-fPIC"; Config = "linux-gcc"; }, },
    },

    Sources = { "src/plugins/breakpoints/breakpoints_plugin.cpp" },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "hex_memory_plugin",
    
    Env = {
        CPPPATH = { "api/include", },
    	CXXOPTS = { { "-fPIC"; Config = "linux-gcc"; }, },
    },

    Sources = { "src/plugins/hex_memory/hex_memory_plugin.cpp" },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "console_plugin",
    
    Env = {
        CPPPATH = { "api/include", },
        CXXOPTS = { { "-fPIC"; Config = "linux-gcc"; }, },
    },

    Sources = { "src/plugins/console/console_plugin.cpp" },

    IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "workspace_plugin",
    
    Env = {
        CPPPATH = { 
			"src/native/external/foundation_lib",
        	"api/include", 
        },
        CXXOPTS = { { "-fPIC"; Config = "linux-gcc"; }, },
    },

    Sources = { "src/plugins/workspace/workspace_plugin.cpp" },

    Depends = { "foundation_lib" },

    Frameworks = { "Cocoa"  },

    Libs = { 
      { "Ws2_32.lib", "psapi.lib", "iphlpapi.lib", "wsock32.lib", "Shell32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib", "Advapi32.lib" ; Config = { "win32-*-*", "win64-*-*" } },
	  -- { "third-party/lib/wx/wx_osx_cocoau_core-3.1", "third-party/lib/wx/wwx_baseu-3.1" ; Config = { "macosx-*-*", "macosx_test-*-*" } },
    },


    IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "c64_vice_plugin",
    
    Env = {
        CPPPATH = { 
			"src/native/external/jansson/include",
			"src/native/external/libuv/include",
			"api/include", 
		},
		
		CCOPTS = {
			{ "-std=c99"; Config = "linux-*"; },
		},

        COPTS = { { "-fPIC"; Config = "linux-gcc"; }, },
		
		CPPDEFS = {
			{ "_XOPEN_SOURCE=600"; Config = "linux-*" },
		},
    },

    Sources = { 
		Glob {
			Dir = "src/addons/c64_vice_debugger",
			Extensions = { ".c", ".h" },
		},
	},

    Libs = { 
      { 
    	"Ws2_32.lib", "psapi.lib", "iphlpapi.lib", "wsock32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib", "Advapi32.lib" ; Config = { "win32-*-*", "win64-*-*" } 
      },
    },

    IdeGenerationHints = { Msvc = { SolutionFolder = "Addons" } },

    Depends = { "jansson", "uv" },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "amiga_uae_plugin",
    
    Env = {
        CPPPATH = { 
			"api/include", 
			"api/src/remote", 
		},
		CCOPTS = {
			{ 
				"-Wno-unused-macros",
				"-Wno-sign-conversion" ; Config = { "macosx-*-*", "linux-*" } },
			{ "-std=c99"; Config = "linux-*"; },
		},

        COPTS = { { "-fPIC"; Config = "linux-gcc"; }, },
		
		CPPDEFS = {
			{ "_XOPEN_SOURCE=600"; Config = "linux-*" },
		},
    },

    Sources = { 
		Glob {
			Dir = "src/addons/amiga_uae_debugger",
			Extensions = { ".c", ".h" },
		},
	},

    Libs = { 
      { 
    	"Ws2_32.lib", "psapi.lib", "iphlpapi.lib", "wsock32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib", "Advapi32.lib" ; Config = { "win32-*-*", "win64-*-*" } 
      },
    },

    IdeGenerationHints = { Msvc = { SolutionFolder = "Addons" } },

    Depends = { "remote_connection", "uv" },
}

-----------------------------------------------------------------------------------------------------------------------

RustSharedLibrary {
	Name = "bitmap_memory",
	CargoConfig = "src/plugins/bitmap_memory/Cargo.toml",
	Sources = { 
		get_rs_src("src/plugins/bitmap_memory"),
		get_rs_src("api/rust/prodbg"),
	}
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "i3_docking",

    Env = { 
        CPPPATH = { 
			"src/native/external/jansson/include",
            "src/plugins/i3_docking/include",
        	"api/include",
        },
        CCOPTS = {
        	{ "-Wno-everything", "-std=c99"; Config = { "macosx-*-*", "macosx_test-*" } },
        	{ "-std=c99"; Config = "linux-*-*" },
        	{ "/wd4267", "/wd4706", "/wd4244", "/wd4701", "/wd4334", "/wd4127"; Config = "win64-*-*" },
        },
    },

    Sources = { 
		Glob {
			Dir = "src/plugins/i3_docking",
			Extensions = { ".c", ".h" },
		},
	},

    IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "dummy_backend_plugin",
    
    Env = {
        CPPPATH = { 
        	"api/include", 
        	"src/native/external", 
        },
    	CXXOPTS = { { "-fPIC"; Config = "linux-gcc"; }, },
    },

    Sources = { "src/plugins/dummy_backend/dummy_backend.c" },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
}


-----------------------------------------------------------------------------------------------------------------------

if native.host_platform == "macosx" then
   Default "lldb_plugin"
end

if native.host_platform == "windows" then
   Default "dbgeng_plugin"
end

Default "registers_plugin"
Default "callstack_plugin"
Default "sourcecode_plugin"
Default "disassembly_plugin"
Default "locals_plugin"
Default "threads_plugin"
Default "breakpoints_plugin"
Default "hex_memory_plugin"
Default "workspace_plugin"
Default "console_plugin"
Default "c64_vice_plugin"
Default "amiga_uae_plugin"
Default "bitmap_memory"
Default "i3_docking"

