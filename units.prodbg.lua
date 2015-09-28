require "tundra.syntax.glob"
require "tundra.syntax.osx-bundle"
require "tundra.path"
require "tundra.util"

local native = require('tundra.native')

-----------------------------------------------------------------------------------------------------------------------

Program {
    Name = "prodbg",

    Env = {
        CPPPATH = { 
			"src/external/remotery/lib",
			"src/external/foundation_lib",
			"src/external/jansson/include",
            "src/external/lua/src",
			"src/external/libuv/include",
            "src/external/bgfx/include", 
            "src/external/bx/include",
            "src/external/stb",
            "src/external/i3wm_docking",
            "src/prodbg", 
        	"api/include",
            "src/frontend",
        },

        PROGOPTS = {
            { "/SUBSYSTEM:WINDOWS", "/DEBUG"; Config = { "win32-*-*", "win64-*-*" } },
        },

        CXXOPTS = { 
			{ 
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
			{ "/EHsc"; Config = "win64-*-*" },
        },

        CCOPTS = {
			{ "-Wno-c11-extensions"; Config = "macosx-clang-*" },
        	{ 
        	  "/wd4201" -- namless struct/union
			  ; Config = "win64-*-*" },
        },

		PROGCOM = {
			{ "-lstdc++"; Config = "linux-gcc-*" },
			{ "-lm -lrt -lpthread -ldl -lX11 -lGL"; Config = "linux-*-*" },
		},
    },

    Sources = { 
        FGlob {
            Dir = "src/prodbg/main",
            Extensions = { ".c", ".cpp", ".m", ".mm", ".h" },
            Filters = {
                { Pattern = "mac"; Config = { "macosx-*-*", "macosx_test-*-*" } },
                { Pattern = "windows"; Config = "win64-*-*" },
                { Pattern = "linux"; Config = "linux-*-*" },
            },

            Recursive = true,
        },
    },

    Depends = { "core", "ui", "api", "session", "jansson", "lua", "remote_api", "stb", 
    			"bgfx", "uv", "imgui", "remotery", "foundation_lib", "scintilla", 
    			"tinyxml2", "i3wm_docking", "capstone" },

    Libs = { 
      { "Ws2_32.lib", "psapi.lib", "iphlpapi.lib", "wsock32.lib", "Shell32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib", "Advapi32.lib" ; Config = { "win32-*-*", "win64-*-*" } },
	  -- { "third-party/lib/wx/wx_osx_cocoau_core-3.1", "third-party/lib/wx/wwx_baseu-3.1" ; Config = { "macosx-*-*", "macosx_test-*-*" } },
    },

    Frameworks = { "Cocoa"  },

	IdeGenerationHints = { Msvc = { SolutionFolder = "ProDBG" } },
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


