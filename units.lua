require "tundra.syntax.glob"
require "tundra.path"
require "tundra.util"

-----------------------------------------------------------------------------------------------------------------------

local function get_src(dir, recursive)
	return FGlob {
		Dir = dir,
		Extensions = { ".cpp", ".c", ".h", ".s", ".m" },
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
    Name = "RemoteAPI",

    Env = { 
        
        CPPPATH = { "API/include" },
        CCOPTS = {
            "-Wno-visibility",
            "-Wno-conversion", 
            "-Wno-pedantic", 
            "-Wno-conversion",
            "-Wno-covered-switch-default",
            "-Wno-unreachable-code",
            "-Wno-bad-function-cast",
            "-Wno-missing-field-initializers",
            "-Wno-float-equal",
            "-Wno-conversion",
            "-Wno-switch-enum",
            "-Wno-format-nonliteral"; Config = "macosx-*-*" 
        },
    },

    Sources = { 
        Glob {
            Dir = "API/src/remote",
            Extensions = { ".c" },
        },
    },
}

-----------------------------------------------------------------------------------------------------------------------
-- Example 6502 emulator

Program {
    Name = "Fake6502",

    Env = {
        CPPPATH = { "API/include" },
        CCOPTS = {
            { 
            "-Wno-conversion", 
            "-Wno-missing-variable-declarations",
            "-Werror", 
            "-Wno-pedantic", 
            "-Wno-conversion",
            "-Wno-missing-field-initializers",
            "-Wno-conversion",
            "-Wno-switch-enum",
            "-Wno-format-nonliteral"; Config = "macosx-*-*" },
        },
    },

    Sources = { 
        Glob {
            Dir = "examples/Fake6502",
            Extensions = { ".c", ".cpp", ".m" },
        },
    },

    Libs = { { "wsock32.lib", "kernel32.lib" ; Config = { "win32-*-*", "win64-*-*" } } },

    Depends = { "RemoteAPI" },
}

-----------------------------------------------------------------------------------------------------------------------
-- Crash Example

Program {
    Name = "crashing_native",

    Sources = { 
        Glob {
            Dir = "examples/CrashingNative",
            Extensions = { ".c" },
        },
    },
}

-----------------------------------------------------------------------------------------------------------------------
---------- Plugins -----------------

SharedLibrary {
    Name = "LLDBPlugin",
    
    Env = {
        CPPPATH = { 
        	"API/include",
            "src/plugins/lldb",
            -- "plugins/LLDB/Frameworks/LLDB.Framework/Headers",
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
            "-Wno-missing-field-initializers"; Config = "macosx-clang-*" },
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
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "SourceCodePlugin",
    
    Env = {
        CPPPATH = { "API/include", },
        SHLIBOPTS = { "-lstdc++"; Config = "macosx-clang-*" },
        CXXCOM = { "-stdlib=libc++"; Config = "macosx-clang-*" },
    },

    Sources = { "src/plugins/sourcecode/SourceCodePlugin.cpp" },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "CallStackPlugin",
    
    Env = {
        CPPPATH = { "API/include", },
        SHLIBOPTS = { "-lstdc++"; Config = "macosx-clang-*" },
        CXXCOM = { "-stdlib=libc++"; Config = "macosx-clang-*" },
    },

    Sources = { "src/plugins/callstack/CallStackPlugin.cpp" },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "Disassembly",
    
    Env = {
        CPPPATH = { "API/include", },
        SHLIBOPTS = { "-lstdc++"; Config = "macosx-clang-*" },
        CXXCOM = { "-stdlib=libc++"; Config = "macosx-clang-*" },
    },

    Sources = { "src/plugins/disassembly/DisassemblyPlugin.cpp" },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "Registers",
    
    Env = {
        CPPPATH = { "API/include", },
        SHLIBOPTS = { "-lstdc++"; Config = "macosx-clang-*" },
        CXXCOM = { "-stdlib=libc++"; Config = "macosx-clang-*" },
    },

    Sources = { "src/plugins/registers/RegistersPlugin.cpp" },
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "Locals",
    
    Env = {
        CPPPATH = { "API/include", },
        SHLIBOPTS = { "-lstdc++"; Config = "macosx-clang-*" },
        CXXCOM = { "-stdlib=libc++"; Config = "macosx-clang-*" },
    },

    Sources = { "src/plugins/locals/LocalsPlugin.cpp" },
}

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
    Name = "glfw",

    Env = { 
        CPPPATH = { 
            "src/External/glfw/src",
        },
        
		CPPDEFS = { 
			{ "_GLFW_WIN32", "_GLFW_WGL", "WIN32"; Config = "win32-*-*" },
			{ "MACOSX", "GLFW_INCLUDE_GLCOREARB", "_GLFW_COCOA", "GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED"; Config = "macosx-*-*" }, 
		},

        CCOPTS = {
        	"-Wno-everything"; Config = "macosx-*-*" 
        },
    },

    Sources = { 
    	get_src("src/External/glfw", true),
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
        	"API/include",
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

    Depends = { "RemoteAPI", "stb", "glfw" },

    Libs = { { "wsock32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib", "Advapi32.lib" ; Config = { "win32-*-*", "win64-*-*" } } },

    Frameworks = { "Cocoa"  },
}

-----------------------------------------------------------------------------------------------------------------------

local native = require('tundra.native')

-- only build LLDBPlugin on Mac

if native.host_platform == "macosx" then
   Default "LLDBPlugin"
end

Default "prodbg"
Default "Registers"
Default "CallStackPlugin"
Default "SourceCodePlugin"
Default "Disassembly"
Default "Fake6502"
Default "Locals"
Default "crashing_native"

