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

RustCrate {
	Name = "prodbg_api",
	CargoConfig = "api/rust/prodbg/Cargo.toml",
	Sources = {
		get_rs_src("api/rust/prodbg"),
	},
}

-----------------------------------------------------------------------------------------------------------------------

RustSharedLibrary {
	Name = "amiga_uae_plugin",
	CargoConfig = "src/addons/amiga_uae_plugin/Cargo.toml",
	Sources = {
		get_rs_src("src/addons/amiga_uae_plugin"),
		get_rs_src("src/crates/amiga_hunk_parser"),
		get_rs_src("src/crates/gdb-remote"),
		get_rs_src("api/rust/prodbg"),
	}
}

-----------------------------------------------------------------------------------------------------------------------

RustSharedLibrary {
	Name = "memory_view_2",
	CargoConfig = "src/plugins/memory_view_2/Cargo.toml",
	Sources = {
		get_rs_src("src/plugins/memory_view_2"),
		get_rs_src("api/rust/prodbg"),
		get_rs_src("api/rust/prodbg_ui"),
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

    Sources = {
        "src/plugins/dummy_backend/dummy_backend.c",
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
}

-----------------------------------------------------------------------------------------------------------------------

if native.host_platform == "macosx" then
   Default "lldb_plugin"
end

--if native.host_platform == "windows" then
--  Default "dbgeng_plugin"
--end

Default "amiga_uae_plugin"
-- Default "memory_view_2"
Default "dummy_backend_plugin"

-- vim: ts=4:sw=4:sts=4

