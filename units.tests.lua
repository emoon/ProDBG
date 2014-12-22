require "tundra.syntax.glob"
require "tundra.path"
require "tundra.util"

-----------------------------------------------------------------------------------------------------------------------

local function Test(params)

	Program {
		Name = params.Name, 

		Env = { 
			CPPPATH = { 
				"src/external/foundation_lib",
				"api/include",
            	"src/external/imgui",
				"src/external/cmocka/include",
				"src/prodbg",
			},

			PROGCOM = {
				{ "-lstdc++", "-coverage"; Config = "macosx_test-clang-*" },
				{ "-lstdc++"; Config = { "macosx-clang-*", "linux-gcc-*" } },
				{ "-lm -lpthread -ldl"; Config = "linux-*-*" },
			},
		},

		Sources = { 
			params.Source,	
		},

		Depends = { "uv", "api", "core", "stb", "remote_api", "cmocka", "session", "ui", "bgfx", "jansson", "imgui", "foundation_lib" },

		Libs = { { "Ws2_32.lib", "shell32.lib", "psapi.lib", "iphlpapi.lib", "wsock32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib", "Advapi32.lib" ; Config = { "win32-*-*", "win64-*-*" } } },

		Frameworks = { "Cocoa"  },
	}

end

-----------------------------------------------------------------------------------------------------------------------

Test({ Name = "core_tests", Source = "src/prodbg/tests/core_tests.cpp" })
Test({ Name = "lldb_tests", Source = "src/prodbg/tests/lldb_tests.cpp" })
Test({ Name = "readwrite_tests", Source = "src/prodbg/tests/readwrite_tests.cpp" })
Test({ Name = "remote_api_tests", Source = "src/prodbg/tests/remote_api_tests.cpp" })
Test({ Name = "session_tests", Source = "src/prodbg/tests/session_tests.cpp" })
Test({ Name = "ui_docking_tests", Source = "src/prodbg/tests/ui_docking_tests.cpp" })
Test({ Name = "ui_tests", Source = "src/prodbg/tests/ui_tests.cpp" })

-----------------------------------------------------------------------------------------------------------------------

Default "core_tests"
Default "lldb_tests"
Default "readwrite_tests"
Default "remote_api_tests"
Default "session_tests"
Default "ui_docking_tests"
Default "ui_tests"

