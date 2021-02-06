require "tundra.path"
require "tundra.util"
require "tundra.syntax.rust-cargo"

local function gen_moc(src)
    return Moc {
        Pass = "GenerateSources",
        Source = src
    }
end

local function gen_uic(src)
    return Uic {
        Pass = "GenerateSources",
        Source = src
    }
end


local native = require('tundra.native')

-----------------------------------------------------------------------------------------------------------------------

local function get_rs_src(dir)
	return Glob {
		Dir = dir,
		Extensions = { ".rs" },
		Recursive = true,
	}
end

local function get_cpp_src(dir)
	return Glob {
		Dir = dir,
		Extensions = { ".cpp", ".c" },
		Recursive = true,
	}
end


-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "vamiga",

    Includes = {
		"src/addons/vamiga/Emulator",
		"src/addons/vamiga/Emulator/Agnus",
		"src/addons/vamiga/Emulator/Agnus/Blitter",
		"src/addons/vamiga/Emulator/Agnus/Copper",
		"src/addons/vamiga/Emulator/CIA",
		"src/addons/vamiga/Emulator/CPU",
		"src/addons/vamiga/Emulator/CPU/Moira",
		"src/addons/vamiga/Emulator/Denise",
		"src/addons/vamiga/Emulator/Drive",
		"src/addons/vamiga/Emulator/Expansion",
		"src/addons/vamiga/Emulator/Files",
		"src/addons/vamiga/Emulator/FileSystems",
		"src/addons/vamiga/Emulator/Foundation",
		"src/addons/vamiga/Emulator/Memory",
		"src/addons/vamiga/Emulator/Paula",
		"src/addons/vamiga/Emulator/Paula/Audio",
		"src/addons/vamiga/Emulator/Paula/DiskController",
		"src/addons/vamiga/Emulator/Paula/UART",
		"src/addons/vamiga/Emulator/Peripherals",
		"src/addons/vamiga/Emulator/RTC",
		"src/addons/vamiga/Emulator/xdm",
		"src/addons/vamiga/Emulator/LogicBoard",
		"src/addons/vamiga/Emulator/Files/RomFiles",
		"src/addons/vamiga/Emulator/Files/DiskFiles",
    },

    Env = {
        CXXOPTS = {
			{ "-Wno-unused-result",
			  "-Wno-reorder",
			  "-Wno-unused-variable",
			  "-std=c++17" ; Config = { "macos-*-*", "linux-*-*" }
            },
        },
    },

    Sources = {
        get_cpp_src("src/addons/vamiga", true),
    }
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "lldb_plugin",

    Env = {
        CPPPATH = {
        	"api/include",
            "src/plugins/lldb",
            "src/external/lldb/include",
        },

        SHLIBOPTS = {
            { "-Fsrc/plugins/lldb/Frameworks", "-rpath src/plugins/lldb/Frameworks", "-lstdc++"; Config = "macos-clang-*" },
            { "-Lsrc/external/lldb/lib/linux", "-Wl,-rpath src/external/lldb/lib/linux"; Config = "linux-*-*" },
        },

        CXXCOM = { "-stdlib=libc++"; Config = "macos-clang-*" },
    },

    Libs = {
		{ "lldb"; Config = "linux-*-*" },
    },

    Sources = {
        Glob {
            Dir = "src/plugins/lldb",
            Extensions = { ".c", ".cpp", ".m" },
        },

    },

    Frameworks = { "LLDB" },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },

	Depends = { "flatbuffers" },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "file",

    Includes = {
        "api/include",
    },

    Sources = {
        get_cpp_src("src/plugins/file")
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },

	Depends = { "flatbuffers" },
}

-----------------------------------------------------------------------------------------------------------------------

local function ViewPlugin(params)
	local shared_lib = SharedLibrary {
        Name = params.Name,

        Env = {
            CPPPATH = {
                "api/include",
                "$(OBJECTDIR)",
            },

            LIBPATH = {
                { "$(QT5_LIB)"; Config = "win64-*-*" },
                { "$(QT5_LIB)"; Config = "linux-*-*" },
            },
        },

        Defines = {
            "QT_NO_DEBUG",
        },

        Sources = params.Sources,

        Libs = {
            { "Qt5Gui.lib", "Qt5Core.lib", "Qt5Widgets.lib"; Config = "win64-*-*" },
        },

        Depends = { "backend_requests" },

		Frameworks = { "Cocoa", "QtWidgets", "QtGui", "QtCore" },

	    IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
	}

	Default(shared_lib)
end

-----------------------------------------------------------------------------------------------------------------------

ViewPlugin {
    Name = "register_view",

    Sources = {
        gen_uic("src/plugins/registers_view/RegisterView.ui"),
        gen_moc("src/plugins/registers_view/registers_view.h"),
        get_cpp_src("src/plugins/registers_view"),
    },
}

-----------------------------------------------------------------------------------------------------------------------

ViewPlugin {
    Name = "memory_view",

    Sources = {
        gen_uic("src/plugins/memory_view/memory_view.ui"),
        gen_moc("src/plugins/memory_view/memory_view.h"),
        gen_moc("src/plugins/memory_view/memory_view_widget.h"),
        get_cpp_src("src/plugins/memory_view"),
    },
}

-----------------------------------------------------------------------------------------------------------------------

ViewPlugin {
    Name = "locals_view",

    Sources = {
        gen_uic("src/plugins/locals_view/locals_view.ui"),
        gen_moc("src/plugins/locals_view/locals_view.h"),
        get_cpp_src("src/plugins/locals_view"),
    },
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

SharedLibrary {
    Name = "dummy_backend",

    Env = {
        CPPPATH = {
        	"api/include",
        	"src/external",
        },
    	CXXOPTS = { { "-fPIC"; Config = "linux-gcc"; }, },
    },

    Sources = {
        "src/plugins/dummy_backend/dummy_backend.cpp",
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Plugins" } },
}

-----------------------------------------------------------------------------------------------------------------------

-- if native.host_platform == "macosx" or native.host_platform == "linux" then
if native.host_platform == "linux" then
   Default "lldb_plugin"
end

--if native.host_platform == "windows" then
--  Default "dbgeng_plugin"
--end

Default "vamiga"
Default "file"
Default "dummy_backend"
Default "register_view"
Default "memory_view"

-- vim: ts=4:sw=4:sts=4

