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
}

-----------------------------------------------------------------------------------------------------------------------

Program {
    Name = "session_tests",

    Env = { 
        CPPPATH = { 
            "api/include",
            "src/external/cmocka/include",
            "src/prodbg",
        },

		PROGCOM = {
			{ "-lstdc++"; Config = { "macosx-clang-*", "linux-gcc-*" } },
			{ "-lm"; Config = "unix-gcc-*" },
		},
    },

    Sources = { 
    	"src/prodbg/tests/session_tests.cpp",
    },

    Depends = { "api", "core", "stb", "remote_api", "cmocka", "session", "ui", "nanovg", "bgfx" },

    Libs = { { "Ws2_32.lib", "psapi.lib", "iphlpapi.lib", "wsock32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib", "Advapi32.lib" ; Config = { "win32-*-*", "win64-*-*" } } },

    Frameworks = { "Cocoa"  },
}

-----------------------------------------------------------------------------------------------------------------------

Program {
    Name = "ui_tests",

    Env = { 
        CPPPATH = { 
            "api/include",
            "src/external/cmocka/include",
            "src/prodbg",
        },

		PROGCOM = {
			{ "-lstdc++"; Config = { "macosx-clang-*", "linux-gcc-*" } },
			{ "-lm"; Config = "unix-gcc-*" },
		},
    },

    Sources = { 
    	"src/prodbg/tests/ui_tests.cpp",
    },

    Depends = { "api", "core", "stb", "ui", "cmocka", "nanovg", "bgfx" },

    Libs = { { "Ws2_32.lib", "psapi.lib", "iphlpapi.lib", "wsock32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib", "Advapi32.lib" ; Config = { "win32-*-*", "win64-*-*" } } },

    Frameworks = { "Cocoa"  },
}

-----------------------------------------------------------------------------------------------------------------------

Default "fake6502"
Default "crashing_native"
Default "session_tests"
Default "ui_tests"

