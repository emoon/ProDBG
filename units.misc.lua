require "tundra.syntax.glob"
require "tundra.path"
require "tundra.util"

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

Default "Fake6502"
Default "crashing_native"

