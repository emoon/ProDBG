require "tundra.syntax.glob"
require "tundra.path"
require "tundra.util"

local function get_c_cpp_src(dir, recursive)
    return Glob {
        Dir = dir,
        Extensions = { ".cpp", ".c", ".h" },
        Recursive = recursive,
}
end

-----------------------------------------------------------------------------------------------------------------------
-- Example 6502 emulator

--[[
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
--]]

-----------------------------------------------------------------------------------------------------------------------
-- Crash Example

Program {
    Name = "crashing_native",

    Sources = {
        get_c_cpp_src("examples/crashing_native", true),
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Misc" } },
}

-----------------------------------------------------------------------------------------------------------------------
-- Unit/integration tests

Program {
    Name = "tests",

    Includes = {
        "src/external/googletest/include",
        "src/external/googletest",
        "src/external/",
        "src",
    },

    Sources = {
        get_c_cpp_src("src/tests", true),
    },

    Env = {
        PROGCOM = {
            { "-lpthread", "-ldl"; Config = "linux-*-*" },
        },
    },

    Depends = { "core" },
}


-- Default "fake6502"
Default "crashing_native"
Default "tests"

-- vim: ts=4:sw=4:sts=4

