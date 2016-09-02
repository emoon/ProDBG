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
    Name = "callstack_plugin",

    Env = {
        CPPPATH = { "api/include", },
    	CXXOPTS = { { "-fPIC"; Config = "linux-gcc"; }, },
    },

    Sources = { "src/plugins/callstack/callstack_plugin.cpp" },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
}

-----------------------------------------------------------------------------------------------------------------------

RustSharedLibrary {
	Name = "locals",
	CargoConfig = "src/plugins/locals/Cargo.toml",
	Sources = {
		get_rs_src("src/plugins/locals"),
		get_rs_src("api/rust/prodbg"),
	}
}

-----------------------------------------------------------------------------------------------------------------------

RustSharedLibrary {
	Name = "threads",
	CargoConfig = "src/plugins/threads/Cargo.toml",
	Sources = {
		get_rs_src("src/plugins/threads"),
		get_rs_src("api/rust/prodbg"),
	}
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
		get_rs_src("src/crates/amiga_hunk_parser"),
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
		get_rs_src("src/crates/amiga_hunk_parser"),
	}
}

-----------------------------------------------------------------------------------------------------------------------

RustSharedLibrary {
	Name = "source_code",
	CargoConfig = "src/plugins/source_code/Cargo.toml",
	Sources = {
		get_rs_src("src/plugins/source_code"),
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
	},
    Depends = { "prodbg_api", "serde_macros", "combo", "char_editor", "number_view" }
}

-----------------------------------------------------------------------------------------------------------------------

RustSharedLibrary {
	Name = "registers_view",
	CargoConfig = "src/plugins/registers_view/Cargo.toml",
	Sources = {
		get_rs_src("src/plugins/registers_view"),
    },
    Depends = { "char_editor", "number_view", "serde_macros", "prodbg_api", "combo" }
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

Default "callstack_plugin"
Default "source_code"
Default "disassembly"
Default "locals"
Default "threads"
Default "breakpoints_plugin"
--Default "workspace_plugin"
Default "console_plugin"
Default "amiga_uae_plugin"
Default "amiga_uae_view_plugin"
Default "bitmap_memory"
Default "memory_view"
Default "registers_view"
Default "dummy_backend_plugin"

-- vim: ts=4:sw=4:sts=4

