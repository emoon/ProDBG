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
				"src/external/jansson/include",
				"src/external/foundation_lib",
				"src/external/minifb/include",
            	"src/external/imgui",
				"src/external/cmocka/include",
				"src/prodbg",
			},

			PROGCOM = {
				{ "-lstdc++", "-coverage"; Config = "macosx_test-clang-*" },
				{ "-lstdc++"; Config = { "macosx-clang-*", "linux-gcc-*" } },
				{ "-lm -lrt -lpthread -ldl -lGL -lX11"; Config = "linux-*-*" },
			},
		},

		Sources = { 
			params.Source,	
		},

		Depends = params.Depends, 

		Libs = {
			{ "Ws2_32.lib", "shell32.lib", "psapi.lib", "iphlpapi.lib", "wsock32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib", "Advapi32.lib" ; Config = { "win32-*-*", "win64-*-*" } },
			{ "X11", "GL" ; Config = { "linux-*-*" } },
		},

		Frameworks = { "Cocoa"  },

		IdeGenerationHints = { Msvc = { SolutionFolder = "Tests" } },
	}

end

-----------------------------------------------------------------------------------------------------------------------

local all_depends = { "uv", "api", "core", "stb", "remote_api", "cmocka", "session", "ui", "bgfx", "jansson", "lua", "imgui", "minifb", "scintilla", "tinyxml2", "foundation_lib", "i3wm_docking" }

-----------------------------------------------------------------------------------------------------------------------

Test({ Name = "core_tests", Source = "src/prodbg/tests/core_tests.cpp", Depends = { "core", "stb", "uv", "cmocka", "foundation_lib", "jansson"} })
Test({ Name = "lldb_tests", Source = "src/prodbg/tests/lldb_tests.cpp", Depends = all_depends})
Test({ Name = "readwrite_tests", Source = "src/prodbg/tests/readwrite_tests.cpp", Depends = all_depends})
Test({ Name = "remote_api_tests", Source = "src/prodbg/tests/remote_api_tests.cpp", Depends = all_depends})
Test({ Name = "session_tests", Source = "src/prodbg/tests/session_tests.cpp", Depends = all_depends})
Test({ Name = "ui_docking_tests", Source = "src/prodbg/tests/ui_docking_tests.cpp", Depends = all_depends})
Test({ Name = "ui_tests", Source = "src/prodbg/tests/ui_tests.cpp", Depends = all_depends})
Test({ Name = "dbgeng_tests", Source = "src/prodbg/tests/dbgeng_tests.cpp", Depends = all_depends })
Test({ Name = "c64_vice_tests", Source = "src/prodbg/tests/c64_vice_tests.cpp", Depends = all_depends })

-----------------------------------------------------------------------------------------------------------------------

Default "core_tests"
Default "lldb_tests"
Default "readwrite_tests"
Default "remote_api_tests"
Default "session_tests"
Default "ui_docking_tests"
Default "ui_tests"
Default "dbgeng_tests"
Default "c64_vice_tests"

