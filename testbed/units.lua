require "tundra.syntax.glob"

Program {
	Name = "Fake6502",

	Sources = { 
		FGlob {
			Dir = "examples/Fake6502",
			Extensions = { ".c", ".cpp", ".m" },
			Filters = {
				{ Pattern = "macosx"; Config = "macosx-*-*" },
				{ Pattern = "windows"; Config = { "win32-*-*", "win64-*-*" } },
			},
		},
	},
}

Program {
	Name = "prodbg-qt5",

	Env = {
		CPPPATH = { ".", 
			"$(QT5)/include/QtWidgets",
			"$(QT5)/include/QtGui",
			"$(QT5)/include", 
		},
		PROGOPTS = {
			{ "/SUBSYSTEM:WINDOWS", "/DEBUG"; Config = { "win32-*-*", "win64-*-*" } },
		},

		CPPDEFS = {
			{ "PRODBG_MAC", Config = "macosx-*-*" },
			{ "PRODBG_WIN"; Config = { "win32-*-*", "win64-*-*" } },
		},

		CPPOPTS = {
			{ "-Werror", "-pedantic-errors", "-Wall"; Config = "macosx-clang-*" },
		},

		PROGCOM = { 
			{ "-F$(QT5)/lib", "-lstdc++"; Config = "macosx-clang-*" },
		},

	},

	Sources = { 
		FGlob {
			Dir = "src/frontend/Qt5",
			Extensions = { ".c", ".cpp", ".m" },
			Filters = {
				{ Pattern = "macosx"; Config = "macosx-*-*" },
				{ Pattern = "windows"; Config = { "win32-*-*", "win64-*-*" } },
			},
		},
	},

	Libs = { { "wsock32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib", "Advapi32.lib" ; Config = "win32-*-*" } },

	Frameworks = { "Cocoa", "QtWidgets", "QtGui", "QtCore"  },
}

Default "Fake6502"
Default "prodbg-qt5"

