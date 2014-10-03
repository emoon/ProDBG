require "tundra.syntax.glob"
require "tundra.syntax.osx-bundle"
require "tundra.path"
require "tundra.util"

local native = require('tundra.native')

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
        	{ "-Wno-everything"; Config = "macosx-*-*" },
        	{ "/wd4244", "/wd4267", "/wd4133", "/wd4047", "/wd4204", "/wd4201", "/wd4701", "/wd4703",
			  "/wd4024", "/wd4100", "/wd4053", "/wd4431", 
			  "/wd4189", "/wd4127"; Config = "win64-*-*" },
        },
    },

    Sources = { 
        Glob {
            Dir = "src/external/stb",
            Extensions = { ".c", ".h" },
        },
    },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "jansson",

    Env = { 
		CPPPATH = { 
			"src/external/jansson/include",
		},

        CCOPTS = {
        	{ "/wd4267", "/wd4706", "/wd4244", "/wd4701", "/wd4334", "/wd4127"; Config = "win64-*-*" },
        },
    },

    Sources = { 
        Glob {
            Dir = "src/external/jansson/src",
            Extensions = { ".c", ".h" },
        },
    },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "uv",

    Env = { 
		CPPPATH = { 
			"src/external/libuv/include",
			"src/external/libuv/src",
		},

        CCOPTS = {
        	{ "-Wno-everything"; Config = "macosx-*-*" },
        	{ "/wd4201", "/wd4127", "/wd4244", "/wd4100", 
			  "/wd4245", "/wd4204", "/wd4701", "/wd4703", "/wd4054",
			  "/wd4702", "/wd4267"; Config = "win64-*-*" },
        },
    },

    Sources = { 
        FGlob {
            Dir = "src/external/libuv/src",
            Extensions = { ".c", ".h" },
            Filters = {
                { Pattern = "unix"; Config = "macosx-*-*" },
                { Pattern = "win"; Config = "win64-*-*" },
            },
        },
    },
}


-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "bgfx",

    Env = { 
        CPPPATH = { 
            "src/external/bgfx/include",
            "src/external/bx/include",
            "src/external/bgfx/3rdparty/khronos",
        },
        
        CXXOPTS = {
			{ "-Wno-variadic-macros", "-Wno-everything" ; Config = "macosx-*-*" },
			{ "/EHsc"; Config = "win64-*-*" },
        },
    },

    Sources = { 
		{ "src/external/bgfx/src/bgfx.cpp",
		  "src/external/bgfx/src/image.cpp",
		  "src/external/bgfx/src/vertexdecl.cpp",
		  "src/external/bgfx/src/renderer_gl.cpp",
		  "src/external/bgfx/src/renderer_null.cpp",
		  "src/external/bgfx/src/renderer_d3d9.cpp", 
		  "src/external/bgfx/src/renderer_d3d11.cpp" }, 
	    { "src/external/bgfx/src/glcontext_wgl.cpp" ; Config = "win64-*-*" },
	    { "src/external/bgfx/src/glcontext_nsgl.mm" ; Config = "macosx-*-*" },
    },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "nanovg",

    Env = { 
        CPPPATH = { 
            "src/external/nanovg",
            "src/external/stb",
            "src/external/bgfx/include",
        },
        
        CXXOPTS = {
        	"-Wno-variadic-macros", 
        	"-Wno-everything" ; Config = "macosx-*-*" 
        },
    },

    Sources = { 
    	get_src("src/external/nanovg", true),
    },
}

-----------------------------------------------------------------------------------------------------------------------

Program {
    Name = "prodbg",

    Env = {
        CPPPATH = { 
			"src/external/jansson/include",
			"src/external/libuv/include",
            "src/external/bgfx/include", 
            "src/external/bx/include",
            "src/external/nanovg",
            "src/external/stb",
            "src/prodbg", 
        	"api/include",
            "src/frontend",
        },

        PROGOPTS = {
            { "/SUBSYSTEM:WINDOWS", "/DEBUG"; Config = { "win32-*-*", "win64-*-*" } },
        },

        CXXOPTS = { 
			{ "-Wno-conversion",
			  "-Wno-gnu-anonymous-struct",
			  "-Wno-global-constructors",
			  "-Wno-nested-anon-types",
			  "-Wno-float-equal",
			  "-Wno-cast-align",
			  "-Wno-exit-time-destructors",
			  "-Wno-format-nonliteral",
			  "-Wno-documentation",	-- Because clang warnings in a bad manner even if the doc is correct
			  "-std=c++11" ; Config = "macosx-clang-*" },
			{ "/EHsc"; Config = "win64-*-*" },
        },

        CCOPTS = {
        	{ 
			  "/wd4201" -- namless struct/union
			  ; Config = "win64-*-*" },
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
                { Pattern = "mac"; Config = "macosx-*-*" },
                { Pattern = "windows"; Config = "win64-*-*" },
            },
        },
    },

    Depends = { "remote_api", "stb", "bgfx", "nanovg", "uv", "jansson" },

    Libs = { { "Ws2_32.lib", "psapi.lib", "iphlpapi.lib", "wsock32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib", "Advapi32.lib" ; Config = { "win32-*-*", "win64-*-*" } } },

    Frameworks = { "Cocoa"  },
}

-----------------------------------------------------------------------------------------------------------------------

local prodbgBundle = OsxBundle 
{
	Depends = { "prodbg" },
	Target = "$(OBJECTDIR)/ProDBG.app",
	InfoPList = "Data/Mac/Info.plist",
	Executable = "$(OBJECTDIR)/prodbg",
	Resources = {
		CompileNib { Source = "data/mac/appnib.xib", Target = "appnib.nib" },
		"data/mac/icon.icns",
	},
}

-----------------------------------------------------------------------------------------------------------------------

if native.host_platform == "macosx" then
	Default(prodbgBundle)
else
	Default "prodbg"
end


