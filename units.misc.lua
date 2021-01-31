require "tundra.syntax.glob"
require "tundra.path"
require "tundra.util"

-----------------------------------------------------------------------------------------------------------------------

local function gen_moc(src)
    return Moc {
        Pass = "GenerateSources",
        Source = src
    }
end

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
            "-Wno-format-nonliteral"; Config = "macos-*-*" },
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
        get_c_cpp_src("src/tests/core", true),
    },

    Env = {
        PROGCOM = {
            { "-lpthread", "-ldl"; Config = "linux-*-*" },
        },
    },

    Depends = { "core" },
}

-----------------------------------------------------------------------------------------------------------------------
-- Unit/integration tests

--[[
Program {
    Name = "qt_core_tests",

    Includes = {
        "api/include",
        "src",
        "src/prodbg",
        "src/prodbg/backend",
        "$(OBJECTDIR)",
        "$(QT5_INC)",
        "$(QT5_INC)/QtCore",
    },

    Sources = {
        gen_moc("src/tests/qt_core_tests/qt_core_tests.cpp"),
        get_c_cpp_src("src/tests/qt_core_tests", true),
    },

    Env = {
        CPPDEFS = {
            "QT_NO_KEYWORDS",
            "QT_NO_CAST_FROM_ASCII",
            "QT_NO_CAST_TO_ASCII",
        },

        PROGCOM = {
            { "-lpthread", "-ldl"; Config = "linux-*-*" },
        },
    },

	Libs = {
		{ "wsock32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib",
		  "Advapi32.lib", "Qt5Core.lib", "Qt5Test.lib"; Config = "win64-*-*" },
		{ "Qt5Test", "Qt5Core" ; Config = "linux-*-*" },
	},

    Frameworks = { "Cocoa", "QtCore" },

    Depends = { "backend", "backend_requests", "core" },
}
--]]

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
-- Default "qt_core_tests"

-- vim: ts=4:sw=4:sts=4

