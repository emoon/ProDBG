require "tundra.syntax.glob"
require "tundra.path"
require "tundra.util"

-----------------------------------------------------------------------------------------------------------------------
-- Example 6502 emulator

Program {
    Name = "fake6502",

    Env = {
        CPPPATH = { "api/include" },
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
            Dir = "examples/fake_6502",
            Extensions = { ".c", ".cpp", ".m" },
        },
    },

    Libs = { { "wsock32.lib", "kernel32.lib" ; Config = { "win32-*-*", "win64-*-*" } } },

    Depends = { "remote_api" },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Misc" } },
}

-----------------------------------------------------------------------------------------------------------------------
-- Crash Example

Program {
    Name = "crashing_native",

    Sources = {
        Glob {
            Dir = "examples/crashing_native",
            Extensions = { ".c" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Misc" } },
}

-----------------------------------------------------------------------------------------------------------------------

Default "fake6502"
Default "crashing_native"

