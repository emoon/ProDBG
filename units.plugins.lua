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

--[[
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
--]]

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

RustSharedLibrary {
	Name = "amiga_uae_plugin",
	CargoConfig = "src/addons/amiga_uae_plugin/Cargo.toml",
	Sources = {
		get_rs_src("src/addons/amiga_uae_plugin"),
		get_rs_src("api/rust/prodbg"),
	}
}

-----------------------------------------------------------------------------------------------------------------------

RustSharedLibrary {
	Name = "amiga_uae_view_plugin",
	CargoConfig = "src/addons/amiga_uae_view_plugin/Cargo.toml",
	Sources = {
		get_rs_src("src/addons/amiga_uae_view_plugin"),
		get_rs_src("api/rust/prodbg"),
	}
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

RustSharedLibrary {
	Name = "memory_view",
	CargoConfig = "src/plugins/memory_view/Cargo.toml",
	Sources = {
		get_rs_src("src/plugins/memory_view"),
		get_rs_src("api/rust/prodbg"),
		get_rs_src("src/helpers/serde_macros"),
	}
}

-----------------------------------------------------------------------------------------------------------------------

RustSharedLibrary {
	Name = "disassembly",
	CargoConfig = "src/plugins/disassembly/Cargo.toml",
	Sources = {
		get_rs_src("src/plugins/disassembly"),
		get_rs_src("api/rust/prodbg"),
	}
}


-----------------------------------------------------------------------------------------------------------------------

--[[
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
--]]

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

--if native.host_platform == "windows" then
--  Default "dbgeng_plugin"
--end

Default "registers_plugin"
Default "callstack_plugin"
Default "sourcecode_plugin"
Default "disassembly"
Default "locals_plugin"
Default "threads_plugin"
Default "breakpoints_plugin"
Default "hex_memory_plugin"
--Default "workspace_plugin"
Default "console_plugin"
Default "amiga_uae_plugin"
Default "amiga_uae_view_plugin"
Default "bitmap_memory"
Default "memory_view"
Default "dummy_backend_plugin"
--Default "i3_docking"

-- vim: ts=4:sw=4:sts=4

