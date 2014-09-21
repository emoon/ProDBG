require "tundra.syntax.glob"
require "tundra.path"
require "tundra.util"

local native = require('tundra.native')

-----------------------------------------------------------------------------------------------------------------------

SharedLibrary {
    Name = "LLDBPlugin",
    
    Env = {
        CPPPATH = { 
        	"API/include",
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

if native.host_platform == "macosx" then
   Default "LLDBPlugin"
end

Default "Registers"
Default "CallStackPlugin"
Default "SourceCodePlugin"
Default "Disassembly"
Default "Locals"

