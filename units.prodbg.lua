require "tundra.syntax.glob"
require "tundra.path"
require "tundra.util"

-----------------------------------------------------------------------------------------------------------------------

local function get_src(dir, recursive)
	return FGlob {
		Dir = dir,
		Extensions = { ".cpp", ".c", ".h", ".s", ".m", ".mm" },
		Filters = {
			{ Pattern = "[/\\]windows[/\\]"; Config = "win32-*" },
			{ Pattern = "[/\\]mac[/\\]"; Config = "mac*-*" },
			{ Pattern = "[/\\]linux[/\\]"; Config = "linux*-*" },
		},
		Recursive = recursive and true or false,
	}
end

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "stb",

    Env = { 
        
        CCOPTS = {
        	"-Wno-everything"; Config = "macosx-*-*" 
        },
    },

    Sources = { 
        Glob {
            Dir = "src/External/stb",
            Extensions = { ".c" },
        },
    },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "bgfx",

    Env = { 
        CPPPATH = { 
            "src/External/bgfx/include",
            "src/External/bx/include",
            "src/External/bgfx/3rdparty/khronos",
        },
        
		CXXDEFS = { 
			{ "BGFX_CONFIG_DEBUG"; Config = "win64-*-debug" },
			{ "BGFX_CONFIG_DEBUG", "BX_PLATFORM_OSX"; Config = "macosx-*-debug" },
		},

        CXXOPTS = {
        	"-Wno-variadic-macros", 
        	"-Wno-everything" ; Config = "macosx-*-*" 
        },
    },

    Sources = { 
		{ "src/External/bgfx/src/bgfx.cpp",
		  "src/External/bgfx/src/image.cpp",
		  "src/External/bgfx/src/vertexdecl.cpp" },

		{ "src/External/bgfx/src/renderer_gl.cpp",
		  "src/External/bgfx/src/glcontext_nsgl.mm" ; Config = "macosx-*-*" },

		{ "src/External/bgfx/entry/renderer_d3d11.cpp"; Config = "win64-*-*" },
    },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "nanovg",

    Env = { 
        CPPPATH = { 
            "src/External/nanovg",
            "src/External/stb",
            "src/External/bgfx/include",
        },
        
        CXXOPTS = {
        	"-Wno-variadic-macros", 
        	"-Wno-everything" ; Config = "macosx-*-*" 
        },
    },

    Sources = { 
    	get_src("src/External/nanovg", true),
    },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "entry",

    Env = { 
        CPPPATH = { 
            "src/External/bgfx/include",
            "src/External/bx/include",
        },
        
		CXXDEFS = { 
			{ "BGFX_CONFIG_DEBUG"; Config = "win64-*-debug" },
			{ "BGFX_CONFIG_DEBUG", "BX_PLATFORM_OSX"; Config = "macosx-*-debug" },
		},

        CXXOPTS = {
        	"-Wno-variadic-macros", 
        	"-Wno-everything" ; Config = "macosx-*-*" 
        },
    },

    Sources = { 
		{ "src/External/bgfx/entry/cmd.cpp",
		  "src/External/bgfx/entry/entry_osx.mm",
		  "src/External/bgfx/entry/entry.cpp",
		  "src/External/bgfx/entry/dbg.cpp",
		  "src/External/bgfx/entry/input.cpp"; Config = "macosx-*-*" },

		{ "src/External/bgfx/entry/cmd.cpp",
		  "src/External/bgfx/entry/entry_windows.cpp",
		  "src/External/bgfx/entry/entry.cpp",
		  "src/External/bgfx/entry/dbg.cpp",
		  "src/External/bgfx/entry/input.cpp"; Config = "win64-*-*" },
    },
}


-----------------------------------------------------------------------------------------------------------------------

Program {
    Name = "prodbg",

    Env = {
        CPPPATH = { 
            "../Arika/include", 
            "src/External/stb",
            "src/prodbg", 
            "src/External/bgfx/include", 
        	"API/include",
            "src/External/bx/include",
            "src/External/nanovg",
            "src/External/bgfx/entry",
            "src/frontend",
        },

        PROGOPTS = {
            { "/SUBSYSTEM:WINDOWS", "/DEBUG"; Config = { "win32-*-*", "win64-*-*" } },
        },

        CPPDEFS = {
            { "PRODBG_MAC", Config = "macosx-*-*" },
            { "PRODBG_WIN", Config = "win64-*-*" },
        },

        CXXOPTS = { { 
        	"-Wno-conversion",
        	"-Wno-gnu-anonymous-struct",
        	"-Wno-global-constructors",
        	"-Wno-nested-anon-types",
        	"-Wno-float-equal",
        	"-Wno-cast-align",
        	"-Wno-exit-time-destructors",
        	"-Wno-format-nonliteral",
            "-Wno-documentation",	-- Because clang warnings in a bad manner even if the doc is correct
            "-std=c++11" ; Config = "macosx-clang-*" },
        },

		PROGCOM = {
			{ "-lstdc++"; Config = "macosx-clang-*" },
		},
    },

    Sources = { 
        FGlob {
            Dir = "src/prodbg",
            Extensions = { ".c", ".cpp", ".m", ".mm", ".h" },
            Filters = {
                { Pattern = "macosx"; Config = "macosx-*-*" },
                { Pattern = "windows"; Config = "win64-*-*" },
            },
        },
    },

    Depends = { "RemoteAPI", "stb", "bgfx", "entry", "nanovg" },

    Libs = { { "wsock32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib", "Advapi32.lib" ; Config = { "win32-*-*", "win64-*-*" } } },

    Frameworks = { "Cocoa"  },
}

-----------------------------------------------------------------------------------------------------------------------

Default "prodbg"

