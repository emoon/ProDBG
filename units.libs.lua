require "tundra.syntax.glob"
require "tundra.syntax.osx-bundle"
require "tundra.path"
require "tundra.util"

-----------------------------------------------------------------------------------------------------------------------
----------------------------------------------- EXTERNAL LIBS --------------------------------------------------------- 
-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "stb",

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
        	{ "/wd4244", "/wd4267", "/wd4133", "/wd4047", "/wd4204", "/wd4201", "/wd4701", "/wd4703",
			  "/wd4024", "/wd4100", "/wd4053", "/wd4431", 
			  "/wd4189", "/wd4127"; Config = "win64-*-*" },
        },
    },

    Sources = { 
        Glob {
            Dir = "src/external/stb",
            Extensions = { ".c", ".h" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "remotery",

    Env = { 
        CCOPTS = {
        	{ "-Wno-everything"; Config = "macosx-*-*" },
        	{ "/wd4267", "/wd4706", "/wd4244", "/wd4701", "/wd4334", "/wd4127"; Config = "win64-*-*" },
        },
    },

    Sources = { 
        Glob {
            Dir = "src/external/remotery/lib",
            Extensions = { ".c", ".h" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "tinyxml2",

    Env = { 
        CXXOPTS = {
        	{ "-Wno-everything"; Config = "macosx-*-*" },
        	{ "/wd4267", "/wd4706", "/wd4244", "/wd4701", "/wd4334", "/wd4127"; Config = "win64-*-*" },
        },
    },

    Sources = { 
        Glob {
            Dir = "src/external/tinyxml2",
            Extensions = { ".cpp", ".h" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "i3wm_docking",

    Env = { 
        CPPPATH = { 
			"src/external/jansson/include",
            "src/external/i3wm_docking/include",
        },
        CCOPTS = {
        	{ "-Wno-everything", "-std=c99"; Config = { "macosx-*-*", "macosx_test-*" } },
        	{ "-std=c99"; Config = "linux-*-*" },
        	{ "/wd4267", "/wd4706", "/wd4244", "/wd4701", "/wd4334", "/wd4127"; Config = "win64-*-*" },
        },
    },

    Sources = { 
        Glob {
            Dir = "src/external/i3wm_docking/src",
            Extensions = { ".c", ".h" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "jansson",

    Env = { 
		CPPPATH = { 
			"src/external/jansson/include",
		},

        CCOPTS = {
        	{ "-Wno-everything"; Config = "macosx-*-*" },
        	{ "/wd4267", "/wd4706", "/wd4244", "/wd4701", "/wd4334", "/wd4127"; Config = "win64-*-*" },
        },
    },

    Sources = { 
        Glob {
            Dir = "src/external/jansson/src",
            Extensions = { ".c", ".h" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "lua",

    Env = { 
        CPPPATH = { 
            "src/external/lua/src",
        },

        CCOPTS = {
            { "-Wno-everything"; Config = "macosx-*-*" },
            { "/wd4267", "/wd4706", "/wd4244", "/wd4701", "/wd4334", "/wd4127"; Config = "win64-*-*" },
        },
    },

    Sources = { 
        Glob {
            Dir = "src/external/lua/src",
            Extensions = { ".c", ".h", ".hpp" },
        },
    },

    IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "foundation_lib",

    Env = { 
		CPPPATH = { 
			"src/external/foundation_lib",
		},

        CCOPTS = {
        	{ "-DFOUNDATION_COMPILE=1", "-funit-at-a-time", "-fstrict-aliasing", "-fno-math-errno", "-ffinite-math-only", "-funsafe-math-optimizations", "-fno-trapping-math", "-ffast-math", "-Wno-missing-braces", "-std=c99"; Config = { "macosx-*-*", "macosx_test-*", "linux-*-*" } },
			{ "-Wno-everything"; Config = { "macosx-*-*", "macosx_test-*" } },
        	{ "/DFOUNDATION_COMPILE=1", "/wd4267", "/wd4706", "/wd4244", "/wd4701", "/wd4334", "/wd4127"; Config = "win64-*-*" },
        	{ "-DBUILD_DEBUG=1"; Config = { "macosx-*-debug", "macosx_test-*-debug", "linux-*-debug" } },
        	{ "-DBUILD_RELEASE=1"; Config = { "macosx-*-release", "macosx_test-*-release", "linux-*-release" } },
        	{ "/DBUILD_DEBUG=1"; Config = "win64-*-debug" },
        	{ "/DBUILD_RELEASE=1"; Config = "win64-*-release" },
        },
    },

    Sources = {
    	FGlob {
            Dir = "src/external/foundation_lib/foundation",
			Extensions = { ".cpp", ".c", ".h", ".s", ".m" },
			Filters = {
				{ Pattern = "[/\\]windows[/\\]"; Config = { "win32-*", "win64-*" } },
				{ Pattern = "[/\\]macosx[/\\]"; Config = "mac*-*" },
				{ Pattern = "[/\\]x11[/\\]"; Config = { "linux-*" } },
			},

			Recursive = true,
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "minifb",

    Env = { 
		CPPPATH = { 
			"src/external/minifb/include",
		},

        CCOPTS = {
        	{ "-Wno-everything"; Config = { "macosx-*-*", "macosx_test-*" } },
			{ "-Wno-missing-braces", "-std=c99"; Config = { "macosx-*-*", "macosx_test-*", "linux-*-*" } },
        	{ "/wd4267", "/wd4706", "/wd4244", "/wd4701", "/wd4334", "/wd4127"; Config = "win64-*-*" },
        },
    },

   Sources = FGlob {
		Dir = "src/external/minifb/src",
		Extensions = { ".cpp", ".c", ".h", ".s", ".m" },
		Filters = {
			{ Pattern = "[/\\]windows[/\\]"; Config = { "win32-*", "win64-*" } },
			{ Pattern = "[/\\]macosx[/\\]"; Config = "mac*-*" },
			{ Pattern = "[/\\]x11[/\\]"; Config = { "linux-*" } },
		},

		Recursive = true,
	},

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "scintilla",

    Env = { 
		CPPPATH = { 
			"src/external/scintilla/include",
			"src/external/scintilla/src/lexlib",
		},

        CXXOPTS = {
        	{ "-DSCI_LEXER", "-Wno-everything", "-Wno-missing-braces" ; Config = { "macosx-*-*", "macosx_test-*" } },
        	{ "-DGTK -DSCI_LEXER", "-Wno-everything", "-Wno-missing-braces" ; Config = { "linux-*-*" } },
        	{ "/DSCI_LEXER", "/wd4267", "/wd4706", "/wd4244", "/wd4701", "/wd4334", "/wd4127"; Config = "win64-*-*" },
        },
    },

    Sources = { 
        Glob {
            Dir = "src/external/scintilla/src",
            Extensions = { ".cxx", ".h" },
        },

		Recursive = true,
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "uv",

    Env = { 
		CPPPATH = { 
			"src/external/libuv/include",
			"src/external/libuv/src",
		},

        CCOPTS = {
        	{ "-Wno-everything"; Config = "macosx-*-*" },
        	{ "/wd4201", "/wd4127", "/wd4244", "/wd4100", 
			  "/wd4245", "/wd4204", "/wd4701", "/wd4703", "/wd4054",
			  "/wd4702", "/wd4267"; Config = "win64-*-*" },
        },
    },

    Sources = { 
    	
    	-- general 
    	
    	{ Glob { 
    		Dir = "src/external/libuv/src", 
    		Extensions = { ".c", ".h" },
    		Recursive = false },
    	},

    	-- Windows

    	{ Glob { 
    		Dir = "src/external/libuv/src/win", 
    		Extensions = { ".c", ".h" },
    		Recursive = false } ; Config = "win64-*-*" 
    	},

    	-- Unix
    	
    	{ Glob { 
    		Dir = "src/external/libuv/src/unix", 
    		Extensions = { ".c", ".h" },
    		Recursive = false } ; Config = { "macosx-*-*", "macosx_test-*", "linux-*-*" }
    	},

    	-- Mac

		{ 
		  "src/external/libuv/src/unix/freebsd/kqueue.c",
		  "src/external/libuv/src/unix/darwin/fsevents.c",
		  "src/external/libuv/src/unix/darwin/darwin-proctitle.c",
		  "src/external/libuv/src/unix/darwin/darwin.c" ; Config = { "macosx-*-*", "macosx_test-*" } },

		-- Linux

		{ 
		  "src/external/libuv/src/unix/linux/linux-core.c",
		  "src/external/libuv/src/unix/linux/linux-inotify.c",
		  "src/external/libuv/src/unix/linux/linux-syscalls.c" ; Config = "linux-*-*" },
	},

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}


-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "bgfx",

    Env = { 
        CPPPATH = { 
		  { "src/external/bx/include/compat/msvc"; Config = "win64-*-*" },
            "src/external/remotery/lib",
            "src/external/bgfx/include",
            "src/external/bx/include",
            "src/external/bgfx/3rdparty/khronos",
        },
        
        CXXOPTS = {
			{ "-Wno-variadic-macros", "-Wno-everything" ; Config = "macosx-*-*" },
			{ "/Isrc/external/bx/include/compat/msvc", "/EHsc"; Config = "win64-*-*" },
        },
    },

    Sources = { 
		{ "src/external/bgfx/src/bgfx.cpp",
		  "src/external/bgfx/src/image.cpp",
		  "src/external/bgfx/src/vertexdecl.cpp",
		  "src/external/bgfx/src/renderdoc.cpp",
		  "src/external/bgfx/src/renderer_gl.cpp",
		  "src/external/bgfx/src/renderer_vk.cpp",
		  "src/external/bgfx/src/renderer_null.cpp",
		  "src/external/bgfx/src/renderer_d3d9.cpp", 
		  "src/external/bgfx/src/renderer_d3d11.cpp", 
		  "src/external/bgfx/src/renderer_d3d12.cpp" }, 
	    { "src/external/bgfx/src/glcontext_wgl.cpp" ; Config = "win64-*-*" },
	    { "src/external/bgfx/src/glcontext_glx.cpp" ; Config = "linux-*-*" },
	    { "src/external/bgfx/src/glcontext_nsgl.mm" ; Config = { "macosx-*-*", "macosx_test-*" } },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "cmocka",

    Env = { 
        CPPPATH = { 
            "src/external/cmocka/include",
        },
        
        CCOPTS = {
       		{ "-Wno-everything" ; Config = "macosx-*-*" },
        	{ "/wd4204", "/wd4701", "/wd4703" ; Config = "win64-*-*" },
       },
    },

    Sources = { 
        Glob {
            Dir = "src/external/cmocka/src",
            Extensions = { ".c", ".h" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "imgui",

    Env = { 
        CPPPATH = { 
			"src/external/scintilla/include",
        },
    },

    Sources = { 
        Glob {
            Dir = "src/external/imgui",
            Extensions = { ".cpp", ".h" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

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
    Name = "angelscript",

    Env = { 
		ASMCOM = "ml64.exe /c /Fo$(@) /W3 /Zi /Ta $(<)",
        CPPPATH = { 
			"src/external/angelscript/angelscript/include",
        },
        
        CXXOPTS = {
			{ "-Wno-variadic-macros", 
			  "-Wno-all",
			  "-Wno-error",
              "-Wno-extra-semi",
              "-Wno-unused-parameter",
			  "-Wno-everything" ; Config = "macosx-*-*" },
			{ "/EHsc"; Config = "win64-*-*" },
        },
    },

    Sources = { {
			"src/external/angelscript/angelscript/source/as_atomic.cpp",
			"src/external/angelscript/angelscript/source/as_builder.cpp",
			"src/external/angelscript/angelscript/source/as_bytecode.cpp",
			"src/external/angelscript/angelscript/source/as_callfunc.cpp",
			"src/external/angelscript/angelscript/source/as_callfunc_x86.cpp",
			"src/external/angelscript/angelscript/source/as_callfunc_x64_gcc.cpp",
			"src/external/angelscript/angelscript/source/as_callfunc_x64_msvc.cpp",
			"src/external/angelscript/angelscript/source/as_callfunc_x64_mingw.cpp",
			"src/external/angelscript/angelscript/source/as_compiler.cpp",
			"src/external/angelscript/angelscript/source/as_configgroup.cpp",
			"src/external/angelscript/angelscript/source/as_context.cpp",
			"src/external/angelscript/angelscript/source/as_datatype.cpp",
			"src/external/angelscript/angelscript/source/as_gc.cpp",
			"src/external/angelscript/angelscript/source/as_generic.cpp",
			"src/external/angelscript/angelscript/source/as_globalproperty.cpp",
			"src/external/angelscript/angelscript/source/as_memory.cpp",
			"src/external/angelscript/angelscript/source/as_module.cpp",
			"src/external/angelscript/angelscript/source/as_objecttype.cpp",
			"src/external/angelscript/angelscript/source/as_outputbuffer.cpp",
			"src/external/angelscript/angelscript/source/as_parser.cpp",
			"src/external/angelscript/angelscript/source/as_restore.cpp",
			"src/external/angelscript/angelscript/source/as_scriptcode.cpp",
			"src/external/angelscript/angelscript/source/as_scriptengine.cpp",
			"src/external/angelscript/angelscript/source/as_scriptfunction.cpp",
			"src/external/angelscript/angelscript/source/as_scriptnode.cpp",
			"src/external/angelscript/angelscript/source/as_scriptobject.cpp",
			"src/external/angelscript/angelscript/source/as_string.cpp",
			"src/external/angelscript/angelscript/source/as_string_util.cpp",
			"src/external/angelscript/angelscript/source/as_thread.cpp",
			"src/external/angelscript/angelscript/source/as_tokenizer.cpp",
			"src/external/angelscript/angelscript/source/as_typeinfo.cpp",
			"src/external/angelscript/angelscript/source/as_variablescope.cpp",
			"src/external/angelscript/add_on/scriptbuilder/scriptbuilder.cpp",
			"src/external/angelscript/add_on/scripthandle/scripthandle.cpp",
			"src/external/angelscript/add_on/scriptstdstring/scriptstdstring.cpp",
			"src/external/angelscript/add_on/scriptstdstring/scriptstdstring_utils.cpp",
			"src/external/angelscript/add_on/weakref/weakref.cpp" },
	      { "src/external/angelscript/angelscript/source/as_callfunc_x64_msvc_asm.asm" ; Config = "win64-*-*" },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "capstone",



    Env = { 
        CPPPATH = { 
			"src/external/capstone/include",
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
            Dir = "src/external/capstone",
            Extensions = { ".h", ".c" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Libs" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "as_debugger",

    Env = { 
        CPPPATH = { 
			"src/external/angelscript/angelscript/include",
			"src/addons/as_debugger",
			"api/include"
        },
        CXXOPTS = {
			"-Wno-all",
			"-Wno-error",
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
            "-Wno-extra-semi",
            "-Wno-unused-parameter",
			"-Wno-everything",
            "-Wno-format-nonliteral"; Config = "macosx-*-*" 
        },
    },

    Sources = { 
        Glob {
            Dir = "src/addons/as_debugger",
            Extensions = { ".h", ".c", ".cpp" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Addons" } },
}

-----------------------------------------------------------------------------------------------------------------------
----------------------------------------------- INTERNAL LIBS --------------------------------------------------------- 
-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "core",

    Env = { 
        CPPPATH = { 
            "src/external/stb",
            "src/external/lua/src",
			"src/external/jansson/include",
			"src/external/foundation_lib",
			"src/external/libuv/include",
			"src/external/i3wm_docking/include",
        	"api/include",
            "src/prodbg",
        },
    },

    Sources = { 
        Glob {
            Dir = "src/prodbg/core",
            Extensions = { ".cpp", ".h" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Libs" } },
}

-----------------------------------------------------------------------------------------------------------------------


StaticLibrary {
    Name = "session",

    Env = { 
        CPPPATH = { 
            "src/external/stb",
			"src/external/libuv/include",
			"src/external/jansson/include",
			"src/external/foundation_lib",
			"src/external/i3wm_docking",
        	"api/include",
            "src/prodbg",
        },

        CXXOPTS = { 
			{ "/EHsc"; Config = "win64-*-*" },
		},
    },

    Sources = { 
        Glob {
            Dir = "src/prodbg/session",
            Extensions = { ".cpp", ".h" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Libs" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "ui",

    Env = { 

        CXXOPTS = {
        	{ "-D__WXOSX_COCOA__",
        	  "-Wno-gnu-anonymous-struct",
			  "-Wno-global-constructors",
			  "-Wno-switch-enum",
			  "-Wno-nested-anon-types",
			  "-Wno-float-equal",
			  "-Wno-cast-align",
			  "-Wno-everything",
			  "-Wno-exit-time-destructors",
			  "-Wno-format-nonliteral"; Config = "macosx-*-*" },
        },

        CPPPATH = { 
			"src/external",
            "src/external/imgui",
            "src/external/bx/include",
            "src/external/bgfx/include",
        	"api/include",
        	"src/external/i3wm_docking",
			"src/external/foundation_lib",
			"src/external/libuv/include",
            "src/external/stb",
            "src/external/jansson/include",
            "src/prodbg",
			"third-party/include",
        },
    },

    Sources = { 
        FGlob {
            Dir = "src/prodbg/ui",
            Extensions = { ".c", ".cpp", ".m", ".mm", ".h", "*.inl" },
            Filters = {
                { Pattern = "mac"; Config = { "macosx-*-*", "macosx_test-*" } },
                { Pattern = "windows"; Config = "win64-*-*" },
                { Pattern = "linux"; Config = "linux-*-*" },
            },

            Recursive = true,
        },

        ShadercFS { Source = "data/shaders/imgui/fs_imgui.sc" },
        ShadercVS { Source = "data/shaders/imgui/vs_imgui.sc" },
        ShadercFS { Source = "data/shaders/ui_pos_color/fs_pos_color.sc" },
        ShadercVS { Source = "data/shaders/ui_pos_color/vs_pos_color.sc" },
        ShadercFS { Source = "data/shaders/ui_pos_tex_r_color/fs_pos_tex_r_color.sc" },
        ShadercVS { Source = "data/shaders/ui_pos_tex_r_color/vs_pos_tex_r_color.sc" },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Libs" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "api",

    Env = { 
        CPPPATH = { 
        	"api/include",
            "src/prodbg",
        },
    },

    Sources = { 
        Glob {
            Dir = "src/prodbg/api",
            Extensions = { ".c", ".cpp", ".h" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Libs" } },
}


