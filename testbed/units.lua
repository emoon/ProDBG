require "tundra.syntax.glob"

-- Used to generate the moc cpp files as needed for .h that uses Q_OBJECT

DefRule {
	Name = "MocGeneration",
	Pass = "GenerateSources",
	Command = "$(QT5)/bin/moc $(<) -o $(@)",

	Blueprint = {
		Source = { Required = true, Type = "string", Help = "Input filename", },
		OutName = { Required = true, Type = "string", Help = "Output filename", },
	},

	Setup = function (env, data)
		return {
			InputFiles    = { data.Source },
			OutputFiles   = { "$(OBJECTDIR)/_generated/" .. data.OutName },
		}
	end,
}

-- Example 6502 emulator

Program {
	Name = "Fake6502",

	Env = {
		CCOPTS = {
			{ "-Wno-conversion", "-Wno-pedantic"; Config = "macosx-*-*" },
		},
	},

	Sources = { 
		Glob {
			Dir = "examples/Fake6502",
			Extensions = { ".c", ".cpp", ".m" },
		},
	},
}

-- Core Lib

StaticLibrary {
	Name = "core",

	Env = { 
		CXXOPTS = {
			{ 
			"-Wno-global-constructors", 
			"-Wno-exit-time-destructors" ; Config = "macosx-clang-*" },
		},
		
		CPPPATH = { "src/frontend", "API" } 
	},

	Sources = { 
		Glob {
			Dir = "src/frontend/core",
			Extensions = { ".c", ".cpp", ".m" },
		},
	},
}

---------- Plugins -----------------

SharedLibrary {
	Name = "LLDBPlugin",
	
	Env = {
		CPPPATH = { 
			"API",
			"plugins/lldb",
			"plugins/lldb/Frameworks/LLDB.Framework/Headers",
		},

		CXXOPTS = {
			{ 
			"-std=c++11", 
			"-Wno-padded",
			"-Wno-documentation",
			"-Wno-unused-parameter",
			"-Wno-missing-prototypes",
			"-Wno-unused-member-function",
			"-Wno-c++98-compat-pedantic" ; Config = "macosx-clang-*" },
		},

		SHLIBOPTS = { 
			{ "-Fplugins/lldb/Frameworks", "-lstdc++"; Config = "macosx-clang-*" },
		},

		CXXCOM = { "-stdlib=libc++"; Config = "macosx-clang-*" },
	},

	Sources = { 
		Glob {
			Dir = "plugins/lldb",
			Extensions = { ".c", ".cpp", ".m" },
		},

	},

	Frameworks = { "LLDB" },
}

------------------------------------


Program {
	Name = "prodbg-qt5",

	Env = {
		CPPPATH = { 
			".", 
			"API",
			"src/frontend",
			"$(QT5)/include/QtWidgets",
			"$(QT5)/include/QtGui",
			"$(QT5)/include/QtCore", 
			"$(QT5)/include", 
		},
		PROGOPTS = {
			{ "/SUBSYSTEM:WINDOWS", "/DEBUG"; Config = { "win32-*-*", "win64-*-*" } },
		},

		CPPDEFS = {
			{ "PRODBG_MAC", Config = "macosx-*-*" },
			{ "PRODBG_WIN"; Config = { "win32-*-*", "win64-*-*" } },
		},

		CXXOPTS = {
			{ 
			--"-mmacosx-version-min=10.7",
			"-std=gnu0x",
			"-std=c++11",
			"-stdlib=libc++",
			"-Wno-padded",
			"-Wno-c++98-compat",
			"-Wno-c++98-compat-pedantic",
			"-Wno-global-constructors",
			"-Wno-long-long",
			"-Wno-unreachable-code",
			"-Wno-float-equal",
			"-Wno-disabled-macro-expansion",
			"-Wno-conversion",
			"-Wno-weak-vtables",
			"-Wno-extra-semi",
			"-Wno-undefined-reinterpret-cast", -- needed for Qt signals :(
			"-Wno-sign-conversion" ; Config = "macosx-clang-*" },
		},

		PROGCOM = { 
			-- hacky hacky
			{ "-F$(QT5)/lib", "-lstdc++", "-rpath tundra-output$(SEP)macosx-clang-debug-default"; Config = "macosx-clang-*" },
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

		MocGeneration {
			Source = "src/frontend/Qt5/Qt5CodeEditor.h",
			OutName = "Qt5CodeEditor_moc.cpp"
		},
		
		MocGeneration {
			Source = "src/frontend/Qt5/Qt5MainWindow.h",
			OutName = "Qt5MainWindow_moc.cpp"
		},

		MocGeneration {
			Source = "src/frontend/Qt5/Qt5HexEditWindow.h",
			OutName = "Qt5HexEditWindow_moc.cpp"
		},

		MocGeneration {
			Source = "src/frontend/Qt5/Qt5HexEditInternal.h",
			OutName = "Qt5HexEditInternal_moc.cpp"
		},

		MocGeneration {
			Source = "src/frontend/Qt5/Qt5HexEditWidget.h",
			OutName = "Qt5HexEditWidget_moc.cpp"
		},

		MocGeneration {
			Source = "src/frontend/Qt5/Qt5DebuggerThread.h",
			OutName = "Qt5DebuggerThread_moc.cpp"
		},
	},

	Depends = { "core" },

	Libs = { { "wsock32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib", "Advapi32.lib" ; Config = "win32-*-*" } },

	Frameworks = { "Cocoa", "QtWidgets", "QtGui", "QtCore", "QtConcurrent"  },
}

Default "LLDBPlugin"
Default "Fake6502"
Default "prodbg-qt5"
