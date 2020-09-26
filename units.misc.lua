require "tundra.syntax.glob"
require "tundra.path"
require "tundra.util"

-----------------------------------------------------------------------------------------------------------------------

local function get_c_cpp_src(dir, recursive)
    return Glob {
        Dir = dir,
        Extensions = { ".cpp", ".c", ".h" },
        Recursive = recursive,
}
end

-----------------------------------------------------------------------------------------------------------------------

DefRule {
	Name = "Flatc",
	Pass = "BuildTools",
	Command = "$(FLATC) --cpp -o $(@) $(<)",
	ImplicitInputs = { "$(FLATC)" },

	Blueprint = {
		Source = { Required = true, Type = "string", Help = "Input filename", },
	},

	Setup = function (env, data)
		return {
			InputFiles    = { data.Source },
			OutputFiles   = { "$(OBJECTDIR)/_generated/" },
		}
	end,
}

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

-----------------------------------------------------------------------------------------------------------------------

Program {
	Name = "flatc",
	Target = "$(FLATC)",

	Pass = "BuildTools",

	SourceDir = "src/external/flatbuffers",

	Includes = {
		"src/external/flatbuffers/include",
		"src/external/flatbuffers",
	},

	Sources = {
		"src/idl_gen_cpp.cpp",
		"src/idl_gen_dart.cpp",
		"src/idl_gen_kotlin.cpp",
		"src/idl_gen_go.cpp",
		"src/idl_gen_js_ts.cpp",
		"src/idl_gen_php.cpp",
		"src/idl_gen_python.cpp",
		"src/idl_gen_lobster.cpp",
		"src/idl_gen_lua.cpp",
		"src/idl_gen_rust.cpp",
		"src/idl_gen_fbs.cpp",
		"src/idl_gen_json_schema.cpp",
		"src/flatc.cpp",
		"src/flatc_main.cpp",
	},

	Env = {
        PROGCOM = {
            { "-lm"; Config = "linux-*-*" },
        },
    },

	Depends = { "flatbuffers" },
}

-----------------------------------------------------------------------------------------------------------------------

Default "flatc"
Default "crashing_native"
Default "tests"

-- vim: ts=4:sw=4:sts=4

