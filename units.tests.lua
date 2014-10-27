require "tundra.syntax.glob"
require "tundra.path"
require "tundra.util"

-----------------------------------------------------------------------------------------------------------------------

local function Test(params)

	Program {
		Name = params.Name, 

		Env = { 
			CPPPATH = { 
				"api/include",
				"src/external/cmocka/include",
				"src/prodbg",
			},

			PROGCOM = {
				{ "-lstdc++"; Config = { "macosx-clang-*", "linux-gcc-*" } },
				{ "-lm -lpthread -ldl"; Config = "linux-*-*" },
			},
		},

		Sources = { 
			params.Source,	
		},

		Depends = { "api", "core", "stb", "remote_api", "cmocka", "session", "ui", "nanovg", "bgfx", "yaml", "uv" },

		Libs = { { "Ws2_32.lib", "psapi.lib", "iphlpapi.lib", "wsock32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib", "Advapi32.lib" ; Config = { "win32-*-*", "win64-*-*" } } },

		Frameworks = { "Cocoa"  },
	}

end

-----------------------------------------------------------------------------------------------------------------------

Test({ Name = "core_tests", Source = "src/prodbg/tests/core_tests.cpp" })
Test({ Name = "session_tests", Source = "src/prodbg/tests/session_tests.cpp" })
Test({ Name = "ui_tests", Source = "src/prodbg/tests/ui_tests.cpp" })
Test({ Name = "lldb_tests", Source = "src/prodbg/tests/lldb_tests.cpp" })
Test({ Name = "readwrite_tests", Source = "src/prodbg/tests/readwrite_tests.cpp" })

-----------------------------------------------------------------------------------------------------------------------

Default "core_tests"
Default "session_tests"
Default "ui_tests"
Default "lldb_tests"
Default "readwrite_tests"

