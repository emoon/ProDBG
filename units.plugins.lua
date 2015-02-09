require "tundra.syntax.glob"
require "tundra.path"
require "tundra.util"

local native = require('tundra.native')

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
            "-Wno-missing-field-initializers"; Config = "macosx-clang-*" },
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
}

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "sourcecode_plugin",
    
    Env = {
        CPPPATH = { "api/include", },
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
Default "breakpoints_plugin"
Default "hex_memory_plugin"
Default "console_plugin"

