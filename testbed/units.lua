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
        
        CPPPATH = { "API" },
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
            Dir = "api/RemoteAPI",
            Extensions = { ".c" },
        },
    },
}

StaticLibrary {
    Name = "CuTest",

    Env = { 
        
        CPPPATH = { "tests/CuTest" },
        CCOPTS = { "-Wno-format-nonliteral"; Config = "macosx-*-*" },
    },

    Sources = { 
        Glob {
            Dir = "tests/CuTest",
            Extensions = { ".c" },
        },
    },
}

-- Example 6502 emulator

Program {
    Name = "Fake6502",

    Env = {
        CPPPATH = { "API" },
        CCOPTS = {
            { 
            "-Wno-conversion", 
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

-- Core Lib

StaticLibrary {
    Name = "core",

    Env = { 
        CXXOPTS = {
            { 
            "-std=gnu0x",
            "-std=c++11",
            "-stdlib=libc++",
            "-Wno-c++98-compat-pedantic",
            "-Wno-global-constructors", 
            "-Wno-gnu",
            "-Wno-documentation",
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
            "plugins/LLDB",
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
            { "-Fplugins/LLDB/Frameworks", "-lstdc++"; Config = "macosx-clang-*" },
        },

        CXXCOM = { "-stdlib=libc++"; Config = "macosx-clang-*" },
    },

    Sources = { 
        Glob {
            Dir = "plugins/LLDB",
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
            "$(QT5)/include",
            "$(QT5)/include/QtWidgets",
            "$(QT5)/include/QtGui",
            "$(QT5)/include/QtCore", 
            "$(QT5)/lib/QtWidgets.framework/Headers", 
            "$(QT5)/lib/QtCore.framework/Headers", 
            "$(QT5)/lib/QtGui.framework/Headers", 
        },

        LIBPATH = {
            { "$(QT5)/lib"; Config = { "win32-*-*", "win64-*-*" } },
        },

        PROGOPTS = {
            { "/SUBSYSTEM:WINDOWS", "/DEBUG"; Config = { "win32-*-*", "win64-*-*" } },
        },

        CPPDEFS = {
            { "PRODBG_MAC", Config = "macosx-*-*" },
            { "PRODBG_WIN", "QT_NO_OPENGL"; Config = { "win32-*-*", "win64-*-*" } },
        },

        CXXOPTS = { { 
            "-F$(QT5)/lib",
            "-std=gnu0x",
            "-std=c++11" ; Config = "macosx-clang-*" },
        },

        PROGCOM = { 
            -- hacky hacky
            { "-F$(QT5)/lib", "-lstdc++", "-rpath tundra-output$(SEP)macosx-clang-debug-default"; Config = "macosx-clang-*" },
        },

    },

    Sources = { 
        FGlob {
            Dir = "src/frontend/Qt5",
            Extensions = { ".c", ".cpp", ".m", ".mm" },
            Filters = {
                { Pattern = "macosx"; Config = "macosx-*-*" },
                { Pattern = "windows"; Config = { "win32-*-*", "win64-*-*" } },
            },
        },

        MocGenerationMulti {
            Glob { 
                Dir = "src/frontend/Qt5", 
                Extensions = { ".h" } 
            }, 
        },
    },

    Depends = { "core", "RemoteAPI" },

    Libs = { { "wsock32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib", "Advapi32.lib",
               "Qt5GUi.lib", "Qt5Core.lib", "Qt5Concurrent.lib", "Qt5Widgets.lib" ; Config = { "win32-*-*", "win64-*-*" } } },

    Frameworks = { "Cocoa", "QtWidgets", "QtGui", "QtCore", "QtConcurrent"  },
}

local native = require('tundra.native')

-- only build LLDBPlugin on Mac

if native.host_platform == "macosx" then
   Default "LLDBPlugin"
end


Program {
    Name = "TestReadWrite",

    Env = {
        CPPPATH = { 
            "tests/CuTest",
            "API",
        },

        CCOPTS = {
            { 
            "-Wno-visibility"; Config = "macosx-*-*" },
        },
    },

    Sources = { "tests/API/ReadWrite.c" },
    Depends = { "CuTest", "RemoteAPI" },
}


------------ Tests ---------------

--Default "TestReadWrite"
Default "prodbg-qt5"
--Default "Fake6502"
