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
            "-Wno-format-nonliteral"; Config = "macosx-*-*"
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
            "-Wno-format-nonliteral"; Config = "macosx-*-*"
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
		-- "-Werror",
		    "-Wno-parentheses",
        	"-Wno-unused-variable",
        	"-Wno-pointer-to-int-cast",
        	"-Wno-int-to-pointer-cast",
        	"-Wno-unused-but-set-variable",
        	"-Wno-return-type",
        	"-Wno-unused-function",
			"-Wno-error=strict-aliasing" ; Config = "linux-*-*" },
        	{ "-Wno-everything"; Config = "macosx-*-*" },
        	{ "/wd4244", "/wd4267", "/wd4133", "/wd4047", "/wd4204", "/wd4201", "/wd4701", "/wd4703", "/wd4090", "/wd4146",
			  "/wd4024", "/wd4100", "/wd4053", "/wd4431",
			  "/wd4189", "/wd4127"; Config = "win64-*-*" },
        },
    },

    Sources = {
        Glob {
            Dir = "src/native/external/tinyexpr",
            Extensions = { ".c", ".h" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "capstone",

    Env = {
        CPPPATH = {
			"src/native/external/capstone/include",
        },

        CCOPTS = {
        	{ "-Wno-everything"; Config = "macosx-*-*" },
        	{ "/wd4267", "/wd4706", "/wd4244", "/wd4701", "/wd4334", "/wd4127"; Config = "win64-*-*" },
        },

		CPPDEFS = {
			{   "CAPSTONE_ARM64_SUPPORT", "CAPSTONE_ARM_SUPPORT", "CAPSTONE_HAS_ARM", "CAPSTONE_HAS_ARM64", "CAPSTONE_HAS_M68K", "CAPSTONE_HAS_MIPS",
				"CAPSTONE_HAS_POWERPC", "CAPSTONE_HAS_SPARC", "CAPSTONE_HAS_SYSZ", "CAPSTONE_HAS_X86", "CAPSTONE_HAS_XCORE", "CAPSTONE_M68K_SUPPORT",
				"CAPSTONE_MIPS_SUPPORT", "CAPSTONE_PPC_SUPPORT", "CAPSTONE_SPARC_SUPPORT", "CAPSTONE_SYSZ_SUPPORT", "CAPSTONE_USE_SYS_DYN_MEM",
				"CAPSTONE_X86_SUPPORT", "CAPSTONE_XCORE_SUPPORT" },
		},
    },

    Sources = {
        Glob {
            Dir = "src/native/external/capstone",
            Extensions = { ".h", ".c" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Libs" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "toolwindowmanager",

    Env = {
       CXXOPTS = {
            { "-isystem $(QT5)/lib/QtWidgets.framework/Headers",
              "-isystem $(QT5)/lib/QtCore.framework/Headers",
              "-isystem $(QT5)/lib/QtGui.framework/Headers"; 
              "-isystem $(QT5)/lib/QtGui.framework/Headers", 
              "-F$(QT5)/lib"; Config = "macosx-*-*" },

            { "-isystem $(QT5)/include/QtWidgets",
              "-isystem $(QT5)/include/QtCore",
              "-isystem $(QT5)/include/QtGui",
              "-isystem $(QT5)/include"; Config = "linux-*-*" },
        },

        CPPPATH = {
            "$(QT5)/include",
            "$(QT5)/include/QtCore",
            "$(QT5)/include/QtGui",
            "$(QT5)/include/QtWidgets",
        },
    },

    Sources = {
        Glob {
            Dir = "src/native/external/toolwindowmanager",
            Extensions = { ".cpp", ".h" },
        },

        gen_moc("src/native/external/toolwindowmanager/ToolWindowManager.h"),
        gen_moc("src/native/external/toolwindowmanager/ToolWindowManagerArea.h"),
        gen_moc("src/native/external/toolwindowmanager/ToolWindowManagerWrapper.h"),
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-- vim: ts=4:sw=4:sts=4

