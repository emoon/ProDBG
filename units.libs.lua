require "tundra.syntax.glob"
require "tundra.syntax.qt"
require "tundra.syntax.osx-bundle"
require "tundra.path"
require "tundra.util"

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

local function gen_uic(src)
    return Uic {
        Pass = "GenerateSources",
        Source = src
    }
end

-----------------------------------------------------------------------------------------------------------------------
----------------------------------------------- EXTERNAL LIBS ---------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "remote_api",

    Env = {
        CPPPATH = { "api/include" },
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
            "-Wno-format-nonliteral"; Config = "macos-*-*"
        },
    },

    Sources = {
        Glob {
            Dir = "api/src/remote",
            Extensions = { ".c" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Libs" } },
}

StaticLibrary {
    Name = "backend_requests",
    Sources = {
        gen_moc("src/prodbg/Backend/IBackendRequests.h"),
    }
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "remote_connection",

    Env = {
        CPPPATH = { "api/include" },
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
            "-Wno-format-nonliteral"; Config = "macos-*-*"
        },
    },

    Sources = {
		"api/src/remote/remote_connection.c",
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Libs" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "tinyexpr",

    Env = {
        CCOPTS = {
        	{
        	"-Wno-overflow",
        	"-Wno-format",
		    "-Wno-parentheses",
        	"-Wno-unused-variable",
        	"-Wno-pointer-to-int-cast",
        	"-Wno-int-to-pointer-cast",
        	"-Wno-unused-but-set-variable",
        	"-Wno-return-type",
        	"-Wno-unused-function",
			"-Wno-error=strict-aliasing" ; Config = "linux-*-*" },
        	{ "-Wno-everything"; Config = "macos-*-*" },
        	{ "/wd4244", "/wd4267", "/wd4133", "/wd4047", "/wd4204", "/wd4201", "/wd4701", "/wd4703", "/wd4090", "/wd4146",
			  "/wd4024", "/wd4100", "/wd4053", "/wd4431",
			  "/wd4189", "/wd4127"; Config = "win64-*-*" },
        },
    },

    Sources = {
        Glob {
            Dir = "src/external/tinyexpr",
            Extensions = { ".c", ".h" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
	Name = "flatbuffers",

	Pass = "BuildTools",

	SourceDir = "src/external/flatbuffers",

	Includes = {
		"src/external/flatbuffers/include",
	},

	Sources = {
		"src/code_generators.cpp",
		"src/idl_parser.cpp",
		"src/idl_gen_text.cpp",
		"src/reflection.cpp",
		"src/util.cpp",
	},
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "capstone",

    Env = {
        CPPPATH = {
			"src/external/capstone/include",
        },

        CCOPTS = {
        	{ "-Wno-everything"; Config = "macos-*-*" },
        	{ "/wd4267", "/wd4706", "/wd4244", "/wd4701", "/wd4334", "/wd4127"; Config = "win64-*-*" },
        },

		CPPDEFS = {
			{   "CAPSTONE_ARM64_SUPPORT", "CAPSTONE_ARM_SUPPORT", "CAPSTONE_HAS_ARM",
				"CAPSTONE_HAS_ARM64", "CAPSTONE_HAS_M68K", "CAPSTONE_HAS_MIPS",
				"CAPSTONE_HAS_POWERPC", "CAPSTONE_HAS_SPARC", "CAPSTONE_HAS_SYSZ",
				"CAPSTONE_HAS_X86", "CAPSTONE_HAS_XCORE", "CAPSTONE_M68K_SUPPORT",
				"CAPSTONE_MIPS_SUPPORT", "CAPSTONE_PPC_SUPPORT", "CAPSTONE_SPARC_SUPPORT",
				"CAPSTONE_SYSZ_SUPPORT", "CAPSTONE_USE_SYS_DYN_MEM",
				"CAPSTONE_X86_SUPPORT", "CAPSTONE_XCORE_SUPPORT" },
		},
    },

    Sources = {
        get_c_cpp_src("src/external/capstone", true)
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Libs" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "toolwindowmanager",

    Env = {
       CXXOPTS = {
            { "-isystem $(QT5_LIB)/QtWidgets.framework/Headers",
              "-isystem $(QT5_LIB)/QtCore.framework/Headers",
              "-isystem $(QT5_LIB)/QtGui.framework/Headers";
              "-isystem $(QT5_LIB)/QtGui.framework/Headers",
              "-F$(QT5)/lib"; Config = "macos-*-*" },
            { "-isystem $(QT5_LIB)" ; Config = "linux-*-*" },
        },

        CPPPATH = {
            "$(QT5_INC)",
            "$(QT5_INC)/QtCore",
            "$(QT5_INC)/QtGui",
            "$(QT5_INC)/QtWidgets",
        },
    },

    Sources = {
        get_c_cpp_src("src/external/toolwindowmanager", true),

        gen_moc("src/external/toolwindowmanager/ToolWindowManager.h"),
        gen_moc("src/external/toolwindowmanager/ToolWindowManagerArea.h"),
        gen_moc("src/external/toolwindowmanager/ToolWindowManagerTabBar.h"),
        gen_moc("src/external/toolwindowmanager/ToolWindowManagerSplitter.h"),
        gen_moc("src/external/toolwindowmanager/ToolWindowManagerWrapper.h"),
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "core",

    Includes = {
        "api/include",
    },

    Sources = {
        get_c_cpp_src("src/core", true),
    },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "edbee",

    Env = {
       CXXOPTS = {
            { "-isystem $(QT5_LIB)/QtWidgets.framework/Headers",
              "-isystem $(QT5_LIB)/QtCore.framework/Headers",
              "-isystem $(QT5_LIB)/QtGui.framework/Headers";
              "-isystem $(QT5_LIB)/QtGui.framework/Headers",
              "-F$(QT5)/lib"; Config = "macos-*-*" },
            { "-isystem $(QT5_LIB)" ; Config = "linux-*-*" },
        },

        CPPPATH = {
            "$(QT5_INC)",
            "$(QT5_INC)/QtCore",
            "$(QT5_INC)/QtGui",
            "$(QT5_INC)/QtWidgets",
            "src/external/edbee-lib/edbee-lib",
            "src/external/edbee-lib/vendor/onig",
            "src/external/edbee-lib/vendor/onig/enc/unicode",
            "src/external/edbee-lib/vendor/qslog/unittest",
            "src/external/edbee-lib/vendor/qslog",
            "$(OBJECTROOT)", "$(OBJECTDIR)",
        },
    },

    Sources = {
        get_c_cpp_src("src/external/edbee-lib/edbee-lib", true),
        get_c_cpp_src("src/external/edbee-lib/vendor/onig", false),
        get_c_cpp_src("src/external/edbee-lib/vendor/onig/enc", false),
        get_c_cpp_src("src/external/edbee-lib/vendor/qslog", false),

        gen_moc("src/external/edbee-lib/vendor/qslog/QsLogWindow.h"),
        gen_uic("src/external/edbee-lib/vendor/qslog/QsLogWindow.ui"),

        gen_moc("src/external/edbee-lib/edbee-lib/edbee/edbee.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/models/chardocument/chartextdocument.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/models/textbuffer.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/models/textdocument.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/models/textdocumentscopes.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/models/texteditorcommandmap.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/models/texteditorconfig.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/models/textlinedata.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/models/textrange.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/models/textsearcher.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/models/textundostack.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/texteditorcontroller.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/texteditorwidget.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/util/test.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/views/components/texteditorautocompletecomponent.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/views/components/texteditorcomponent.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/views/components/textmargincomponent.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/views/texteditorscrollarea.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/views/textrenderer.h"),
        gen_moc("src/external/edbee-lib/edbee-lib/edbee/views/texttheme.h"),
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-- vim: ts=4:sw=4:sts=4

