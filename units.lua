require "tundra.syntax.glob"
require "tundra.path"
require "tundra.util"

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
            { "-Fsrc/plugins/lldb/Frameworks", "-lstdc++"; Config = "macosx-clang-*" },
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

SharedLibrary {
    Name = "Registers",
    
    Env = {
        CPPPATH = { "API/include", },
        SHLIBOPTS = { "-lstdc++"; Config = "macosx-clang-*" },
        CXXCOM = { "-stdlib=libc++"; Config = "macosx-clang-*" },
    },

    Sources = { "src/plugins/registers/RegistersPlugin.cpp" },
}

------------------------------------


Program {
    Name = "prodbg",

    Env = {
        CPPPATH = { 
            "src/prodbg", 
        	"API/include",
        },

        PROGOPTS = {
            { "/SUBSYSTEM:WINDOWS", "/DEBUG"; Config = { "win32-*-*", "win64-*-*" } },
        },

        CPPDEFS = {
            { "PRODBG_MAC", Config = "macosx-*-*" },
            { "PRODBG_WIN", Config = "win64-*-*" },
        },

        CXXOPTS = { { 
        	-- Mark Qt headers as system to silence all the warnings from them
            "-Wno-documentation",	-- Because clang warnings in a bad manner even if the doc is correct
            "-std=c++11" ; Config = "macosx-clang-*" },
        },

        PROGCOM = { 
            -- hacky hacky
            { "-lstdc++", "-rpath tundra-output$(SEP)macosx-clang-debug-default"; Config = "macosx-clang-*" },
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

    Depends = { "RemoteAPI" },

    Libs = { "wsock32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib", "Advapi32.lib"; Config = "win64-*-*" },
}

local native = require('tundra.native')

-- only build LLDBPlugin on Mac

if native.host_platform == "macosx" then
   Default "LLDBPlugin"
end

Default "prodbg"
Default "Registers"
