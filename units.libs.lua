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
            Dir = "src/native/external/stb",
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
            Dir = "src/native/external/remotery/lib",
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
            Dir = "src/native/external/tinyxml2",
            Extensions = { ".cpp", ".h" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "jansson",

    Env = { 
		CPPPATH = { 
			"src/native/external/jansson/include",
		},

        CCOPTS = {
        	{ "-Wno-everything"; Config = "macosx-*-*" },
        	{ "/wd4267", "/wd4706", "/wd4244", "/wd4701", "/wd4334", "/wd4127"; Config = "win64-*-*" },
        },
    },

    Sources = { 
        Glob {
            Dir = "src/native/external/jansson/src",
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
            "src/native/external/lua/src",
        },

        CCOPTS = {
            { "-Wno-everything"; Config = "macosx-*-*" },
            { "/wd4267", "/wd4706", "/wd4244", "/wd4701", "/wd4334", "/wd4127"; Config = "win64-*-*" },
        },
    },

    Sources = { 
        Glob {
            Dir = "src/native/external/lua/src",
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
			"src/native/external/foundation_lib",
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
            Dir = "src/native/external/foundation_lib/foundation",
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
    Name = "scintilla",

    Env = { 
		CPPPATH = { 
			"src/native/external/scintilla/include",
			"src/native/external/scintilla/src/lexlib",
		},

        CXXOPTS = {
        	{ "-DSCI_LEXER", "-Wno-everything", "-Wno-missing-braces" ; Config = { "macosx-*-*", "macosx_test-*" } },
        	{ "-DGTK -DSCI_LEXER", "-Wno-everything", "-Wno-missing-braces" ; Config = { "linux-*-*" } },
        	{ "/DSCI_LEXER", "/wd4267", "/wd4706", "/wd4244", "/wd4701", "/wd4334", "/wd4127"; Config = "win64-*-*" },
        },
    },

    Sources = { 
        Glob {
            Dir = "src/native/external/scintilla/src",
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
			"src/native/external/libuv/include",
			"src/native/external/libuv/src",
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
    		Dir = "src/native/external/libuv/src", 
    		Extensions = { ".c", ".h" },
    		Recursive = false },
    	},

    	-- Windows

    	{ Glob { 
    		Dir = "src/native/external/libuv/src/win", 
    		Extensions = { ".c", ".h" },
    		Recursive = false } ; Config = "win64-*-*" 
    	},

    	-- Unix
    	
    	{ Glob { 
    		Dir = "src/native/external/libuv/src/unix", 
    		Extensions = { ".c", ".h" },
    		Recursive = false } ; Config = { "macosx-*-*", "macosx_test-*", "linux-*-*" }
    	},

    	-- Mac

		{ 
		  "src/native/external/libuv/src/unix/freebsd/kqueue.c",
		  "src/native/external/libuv/src/unix/darwin/fsevents.c",
		  "src/native/external/libuv/src/unix/darwin/darwin-proctitle.c",
		  "src/native/external/libuv/src/unix/darwin/darwin.c" ; Config = { "macosx-*-*", "macosx_test-*" } },

		-- Linux

		{ 
		  "src/native/external/libuv/src/unix/linux/linux-core.c",
		  "src/native/external/libuv/src/unix/linux/linux-inotify.c",
		  "src/native/external/libuv/src/unix/linux/linux-syscalls.c" ; Config = "linux-*-*" },
	},

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}


-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "bgfx",

    Env = { 
        CPPPATH = { 
		  { "src/native/external/bx/include/compat/msvc"; Config = "win64-*-*" },
            "src/native/external/remotery/lib",
            "src/native/external/bgfx/include",
            "src/native/external/bx/include",
            "src/native/external/bgfx/3rdparty/khronos",
        },
        
        CXXOPTS = {
			{ "-Wno-variadic-macros", "-Wno-everything" ; Config = "macosx-*-*" },
			{ "/Isrc/native/external/bx/include/compat/msvc", "/EHsc"; Config = "win64-*-*" },
        },
    },

    Sources = { 
		{ "src/native/external/bgfx/src/bgfx.cpp",
		  "src/native/external/bgfx/src/image.cpp",
		  "src/native/external/bgfx/src/vertexdecl.cpp",
		  "src/native/external/bgfx/src/renderdoc.cpp",
		  "src/native/external/bgfx/src/renderer_gl.cpp",
		  "src/native/external/bgfx/src/renderer_vk.cpp",
		  "src/native/external/bgfx/src/renderer_null.cpp",
		  "src/native/external/bgfx/src/renderer_d3d9.cpp", 
		  "src/native/external/bgfx/src/renderer_d3d11.cpp", 
		  "src/native/external/bgfx/src/renderer_d3d12.cpp" }, 
	    { "src/native/external/bgfx/src/glcontext_wgl.cpp" ; Config = "win64-*-*" },
	    { "src/native/external/bgfx/src/glcontext_glx.cpp" ; Config = "linux-*-*" },
	    { "src/native/external/bgfx/src/glcontext_nsgl.mm" ; Config = { "macosx-*-*", "macosx_test-*" } },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "cmocka",

    Env = { 
        CPPPATH = { 
            "src/native/external/cmocka/include",
        },
        
        CCOPTS = {
       		{ "-Wno-everything" ; Config = "macosx-*-*" },
        	{ "/wd4204", "/wd4701", "/wd4703" ; Config = "win64-*-*" },
       },
    },

    Sources = { 
        Glob {
            Dir = "src/native/external/cmocka/src",
            Extensions = { ".c", ".h" },
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "imgui",

    Env = { 
        CXXOPTS = {
        	{ "-Wno-everything"; Config = "macosx-*-*" },
        },
        CPPPATH = { 
			"src/native/external/scintilla/include",
        },
    },

    Sources = { 
        Glob {
            Dir = "src/native/external/imgui",
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
			"src/native/external/angelscript/angelscript/include",
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
			"src/native/external/angelscript/angelscript/source/as_atomic.cpp",
			"src/native/external/angelscript/angelscript/source/as_builder.cpp",
			"src/native/external/angelscript/angelscript/source/as_bytecode.cpp",
			"src/native/external/angelscript/angelscript/source/as_callfunc.cpp",
			"src/native/external/angelscript/angelscript/source/as_callfunc_x86.cpp",
			"src/native/external/angelscript/angelscript/source/as_callfunc_x64_gcc.cpp",
			"src/native/external/angelscript/angelscript/source/as_callfunc_x64_msvc.cpp",
			"src/native/external/angelscript/angelscript/source/as_callfunc_x64_mingw.cpp",
			"src/native/external/angelscript/angelscript/source/as_compiler.cpp",
			"src/native/external/angelscript/angelscript/source/as_configgroup.cpp",
			"src/native/external/angelscript/angelscript/source/as_context.cpp",
			"src/native/external/angelscript/angelscript/source/as_datatype.cpp",
			"src/native/external/angelscript/angelscript/source/as_gc.cpp",
			"src/native/external/angelscript/angelscript/source/as_generic.cpp",
			"src/native/external/angelscript/angelscript/source/as_globalproperty.cpp",
			"src/native/external/angelscript/angelscript/source/as_memory.cpp",
			"src/native/external/angelscript/angelscript/source/as_module.cpp",
			"src/native/external/angelscript/angelscript/source/as_objecttype.cpp",
			"src/native/external/angelscript/angelscript/source/as_outputbuffer.cpp",
			"src/native/external/angelscript/angelscript/source/as_parser.cpp",
			"src/native/external/angelscript/angelscript/source/as_restore.cpp",
			"src/native/external/angelscript/angelscript/source/as_scriptcode.cpp",
			"src/native/external/angelscript/angelscript/source/as_scriptengine.cpp",
			"src/native/external/angelscript/angelscript/source/as_scriptfunction.cpp",
			"src/native/external/angelscript/angelscript/source/as_scriptnode.cpp",
			"src/native/external/angelscript/angelscript/source/as_scriptobject.cpp",
			"src/native/external/angelscript/angelscript/source/as_string.cpp",
			"src/native/external/angelscript/angelscript/source/as_string_util.cpp",
			"src/native/external/angelscript/angelscript/source/as_thread.cpp",
			"src/native/external/angelscript/angelscript/source/as_tokenizer.cpp",
			"src/native/external/angelscript/angelscript/source/as_typeinfo.cpp",
			"src/native/external/angelscript/angelscript/source/as_variablescope.cpp",
			"src/native/external/angelscript/add_on/scriptbuilder/scriptbuilder.cpp",
			"src/native/external/angelscript/add_on/scripthandle/scripthandle.cpp",
			"src/native/external/angelscript/add_on/scriptstdstring/scriptstdstring.cpp",
			"src/native/external/angelscript/add_on/scriptstdstring/scriptstdstring_utils.cpp",
			"src/native/external/angelscript/add_on/weakref/weakref.cpp" },
	      { "src/native/external/angelscript/angelscript/source/as_callfunc_x64_msvc_asm.asm" ; Config = "win64-*-*" },
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
    Name = "as_debugger",

    Env = { 
        CPPPATH = { 
			"src/native/external/angelscript/angelscript/include",
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
    Name = "ui",

    Env = { 
    	CCOPTS = {
			{ "-Wno-objc-interface-ivars",
			  "-Wno-direct-ivar-access",
			  "-Wno-nullable-to-nonnull-conversion",
			  "-Wno-format-nonliteral"; Config = "macosx-*-*" },
		},

        CXXOPTS = {
        	{ 

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
			"src/native/external",
            "src/native/external/imgui",
            "src/native/external/bx/include",
            "src/native/external/bgfx/include",
        	"api/include",
        	"src/native/external/i3wm_docking",
			-- "src/native/external/foundation_lib",
			"src/native/external/libuv/include",
            "src/native/external/stb",
            "src/native/external/jansson/include",
            "src/prodbg",
			"third-party/include",
        },
    },

    Sources = { 
        FGlob {
            Dir = "src/native/ui",
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

--[[

StaticLibrary {
    Name = "main_lib",

    Env = {
        CPPPATH = { 
			"src/native/external/remotery/lib",
			"src/native/external/foundation_lib",
			"src/native/external/jansson/include",
            "src/native/external/lua/src",
			"src/native/external/libuv/include",
            "src/native/external/bgfx/include", 
            "src/native/external/bx/include",
            "src/native/external/stb",
            "src/native/external/i3wm_docking",
            "src/native", 
        	"api/include",
            "src/frontend",
        },

        PROGOPTS = {
            { "/SUBSYSTEM:WINDOWS", "/DEBUG"; Config = { "win32-*-*", "win64-*-*" } },
        },

        CXXOPTS = { 
			{ 
			  "-Wno-conversion",
			  "-Wno-gnu-anonymous-struct",
			  "-Wno-global-constructors",
			  "-Wno-nested-anon-types",
			  "-Wno-float-equal",
			  "-Wno-cast-align",
			  "-Wno-exit-time-destructors",
			  "-Wno-format-nonliteral",
			  "-Wno-documentation",	-- Because clang warnings in a bad manner even if the doc is correct
			  "-std=c++11" ; Config = "macosx-clang-*" },
			{ "/EHsc"; Config = "win64-*-*" },
        },

        CCOPTS = {
			{ "-Wno-c11-extensions"; Config = "macosx-clang-*" },
        	{ 
        	  "/wd4201" -- namless struct/union
			  ; Config = "win64-*-*" },
        },

		PROGCOM = {
			{ "-lstdc++"; Config = "linux-gcc-*" },
			{ "-lm -lrt -lpthread -ldl -lX11 -lGL"; Config = "linux-*-*" },
		},
    },

    Sources = { 
        FGlob {
            Dir = "src/native/main_lib",
            Extensions = { ".c", ".cpp", ".m", ".mm", ".h" },
            Filters = {
                { Pattern = "mac"; Config = { "macosx-*-*", "macosx_test-*-*" } },
                { Pattern = "windows"; Config = "win64-*-*" },
                { Pattern = "linux"; Config = "linux-*-*" },
            },

            Recursive = true,
        },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Libs" } },
}

--]]



