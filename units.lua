require "tundra.syntax.glob"
require "tundra.path"
require "tundra.util"

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

-- Used to send a list of header files 

local function MocGenerationMulti(sources)
 local result = {}
 for _, src in ipairs(tundra.util.flatten(sources)) do
   result[#result + 1] = MocGeneration { Source = src, OutName = tundra.path.get_filename_base(src) .. "_moc.cpp" }
 end
 return result
end

StaticLibrary {
    Name = "RemoteAPI",

    Env = { 
        
        CPPPATH = { "API/include" },
        CCOPTS = {
            "-Wno-visibility",
            "-Wno-conversion", 
            "-Wno-pedantic", 
            "-Wno-conversion",
            "-Wno-covered-switch-default",
            "-Wno-unreachable-code",
            "-Wno-bad-function-cast",
            "-Wno-missing-field-initializers",
            "-Wno-float-equal",
            "-Wno-conversion",
            "-Wno-switch-enum",
            "-Wno-format-nonliteral"; Config = "macosx-*-*" 
        },
    },

    Sources = { 
        Glob {
            Dir = "API/src/remote",
            Extensions = { ".c" },
        },
    },
}

-- Example 6502 emulator

Program {
    Name = "Fake6502",

    Env = {
        CPPPATH = { "API/include" },
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
            Dir = "examples/Fake6502",
            Extensions = { ".c", ".cpp", ".m" },
        },
    },

    Libs = { { "wsock32.lib", "kernel32.lib" ; Config = { "win32-*-*", "win64-*-*" } } },

    Depends = { "RemoteAPI" },
}

-- Crash Example

Program {
    Name = "crashing_native",

    Sources = { 
        Glob {
            Dir = "examples/CrashingNative",
            Extensions = { ".c" },
        },
    },
}

---------- Plugins -----------------

SharedLibrary {
    Name = "LLDBPlugin",
    
    Env = {
        CPPPATH = { 
        	"API/include",
            "src/plugins/lldb",
            -- "plugins/LLDB/Frameworks/LLDB.Framework/Headers",
        },

        CXXOPTS = { { 
            "-std=c++11", 
            "-Wno-padded",
            "-Wno-documentation",
            "-Wno-unused-parameter",
            "-Wno-missing-prototypes",
            "-Wno-unused-member-function",
            "-Wno-switch",
            "-Wno-switch-enum",
            "-Wno-c++98-compat-pedantic",
            "-Wno-missing-field-initializers"; Config = "macosx-clang-*" },
        },

        SHLIBOPTS = { 
            { "-Fsrc/plugins/lldb/Frameworks", "-rpath src/plugins/lldb/Frameworks", "-lstdc++"; Config = "macosx-clang-*" },
        },

        CXXCOM = { "-stdlib=libc++"; Config = "macosx-clang-*" },
    },

    Sources = { 
        Glob {
            Dir = "src/plugins/lldb",
            Extensions = { ".c", ".cpp", ".m" },
        },

    },

    Frameworks = { "LLDB" },
}

------------------------------------

SharedLibrary {
    Name = "Disassembly",
    
    Env = {
        CPPPATH = { "API/include", },
        SHLIBOPTS = { "-lstdc++"; Config = "macosx-clang-*" },
        CXXCOM = { "-stdlib=libc++"; Config = "macosx-clang-*" },
    },

    Sources = { "src/plugins/disassembly/DisassemblyPlugin.cpp" },
}

------------------------------------

SharedLibrary {
    Name = "Registers",
    
    Env = {
        CPPPATH = { "API/include", },
        SHLIBOPTS = { "-lstdc++"; Config = "macosx-clang-*" },
        CXXCOM = { "-stdlib=libc++"; Config = "macosx-clang-*" },
    },

    Sources = { "src/plugins/registers/RegistersPlugin.cpp" },
}

------------------------------------

SharedLibrary {
    Name = "Locals",
    
    Env = {
        CPPPATH = { "API/include", },
        SHLIBOPTS = { "-lstdc++"; Config = "macosx-clang-*" },
        CXXCOM = { "-stdlib=libc++"; Config = "macosx-clang-*" },
    },

    Sources = { "src/plugins/locals/LocalsPlugin.cpp" },
}

------------------------------------


Program {
    Name = "prodbg",

    Env = {
        CPPPATH = { 
            "../Arika/include", 
            "src/prodbg", 
        	"API/include",
            "src/frontend",
            "$(QT5)/include",
            "$(QT5)/include/QtWidgets",
            "$(QT5)/include/QtGui",
            "$(QT5)/include/QtCore", 
            "$(QT5)/lib/QtWidgets.framework/Headers", 
            "$(QT5)/lib/QtCore.framework/Headers", 
            "$(QT5)/lib/QtGui.framework/Headers", 
        },

        PROGOPTS = {
            { "/SUBSYSTEM:WINDOWS", "/DEBUG"; Config = { "win32-*-*", "win64-*-*" } },
        },

		LIBPATH = {
			{ "$(QT5)/lib"; Config = { "win32-*-*", "win64-*-*" } },
		},

        CPPDEFS = {
            { "PRODBG_MAC", Config = "macosx-*-*" },
            { "PRODBG_WIN", Config = "win64-*-*" },
        },

        CXXOPTS = { { 
        	-- Mark Qt headers as system to silence all the warnings from them
            "-isystem $(QT5)",
            "-isystem $(QT5)/include/QtCore",
            -- "-isystem $(QT5)/lib/QtWidgets.framework/Headers", 
            -- "-isystem $(QT5)/lib/QtCore.framework/Headers", 
            -- "-isystem $(QT5)/lib/QtGui.framework/Headers", 
            -- "-isystem $(QT5)/lib/QtWidgets.framework/Versions/5/Headers", 
            -- "-isystem $(QT5)/lib/QtCore.framework/Versions/5/Headers", 
            -- "-isystem $(QT5)/lib/QtGui.framework/Versions/5/Headers", 
            "-F$(QT5)/lib",
            "-Wno-disabled-macro-expansion", -- meh!
            "-Wno-sign-conversion", -- meh
            "-Wno-unreachable-code", -- meh
            "-Wno-float-equal",
            "-Wno-nested-anon-types",
            "-Wno-deprecated",
            "-Wno-documentation",	-- Because clang warnings in a bad manner even if the doc is correct
            "-std=c++11" ; Config = "macosx-clang-*" },
        },

        PROGCOM = { 
            -- hacky hacky
            { "-F$(QT5)/lib", "-lstdc++", "-rpath tundra-output$(SEP)macosx-clang-debug-default"; Config = "macosx-clang-*" },
        },

    },

    Sources = { 
        FGlob {
            Dir = "src/prodbg",
            Extensions = { ".c", ".cpp", ".m", ".mm", ".h" },
            Filters = {
                { Pattern = "macosx"; Config = "macosx-*-*" },
                { Pattern = "windows"; Config = "win64-*-*" },
            },
        },

        MocGenerationMulti {
            Glob { 
                Dir = "src/prodbg/ui", 
                Extensions = { ".h" } 
            }, 
        },
    },

    Depends = { "RemoteAPI" },

    Libs = { { "wsock32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib", "Advapi32.lib",
               "Qt5GUi.lib", "Qt5Core.lib", "Qt5Concurrent.lib", "Qt5Widgets.lib" ; Config = { "win32-*-*", "win64-*-*" } } },

    Frameworks = { "Cocoa", "QtWidgets", "QtGui", "QtCore", "QtConcurrent"  },
}

local native = require('tundra.native')

-- only build LLDBPlugin on Mac

if native.host_platform == "macosx" then
   Default "LLDBPlugin"
end

Default "prodbg"
Default "Registers"
Default "Disassembly"
Default "Fake6502"
Default "Locals"
Default "crashing_native"

