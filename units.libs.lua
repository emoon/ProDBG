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
    Name = "bgfx_native",

    Env = {
        CPPPATH = {
		  {
		  	{ "src/native/external/bx/include/compat/msvc",
		  	  "src/native/external/bgfx/3rdparty/dxsdk/include" } ; Config = "win64-*-*" },
          { "src/native/external/bx/include/compat/osx" ; Config = "macosx-*-*" },
            "src/native/external/remotery/lib",
            "src/native/external/bgfx/include",
            "src/native/external/bx/include",
            "src/native/external/bgfx/3rdparty/khronos",
        },

        CXXOPTS = {
			{ "-Wno-variadic-macros", "-Wno-everything" ; Config = "macosx-*-*" },
			{ "/Isrc/native/external/bx/include/compat/msvc",
			"/EHsc"; Config = "win64-*-*" },
        },
    },

    Sources = {
		{ "src/native/external/bgfx/src/bgfx.cpp",
		  "src/native/external/bgfx/src/image.cpp",
		  "src/native/external/bgfx/src/vertexdecl.cpp",
		  "src/native/external/bgfx/src/debug_renderdoc.cpp",
		  "src/native/external/bgfx/src/topology.cpp",
		  "src/native/external/bgfx/src/shader_dxbc.cpp",
		  "src/native/external/bgfx/src/renderer_gl.cpp",
		  "src/native/external/bgfx/src/renderer_vk.cpp",
		  "src/native/external/bgfx/src/renderer_null.cpp",
		  "src/native/external/bgfx/src/renderer_d3d9.cpp",
		  "src/native/external/bgfx/src/renderer_d3d11.cpp",
		  "src/native/external/bgfx/src/renderer_d3d12.cpp" },
        { "src/native/external/bgfx/src/renderer_mtl.mm" ; Config = { "macosx-*-*", "macosx_test-*" } },
	    { "src/native/external/bgfx/src/glcontext_wgl.cpp" ; Config = "win64-*-*" },
	    { "src/native/external/bgfx/src/glcontext_glx.cpp" ; Config = "linux-*-*" },
	    { "src/native/external/bgfx/src/glcontext_nsgl.mm" ; Config = { "macosx-*-*", "macosx_test-*" } },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "External" } },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "imgui",

    Env = {
    	CPPDEFS = { "IMGUI_INCLUDE_IMGUI_USER_H" },
        CXXOPTS = {
        	{ "-Wno-everything"; Config = "macosx-*-*" },
        },
        CPPPATH = {
			"src/native/external/scintilla/include",
			"src/native/ui",
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
----------------------------------------------- INTERNAL LIBS ---------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "ui",

    Env = {
    	CPPDEFS = { "IMGUI_INCLUDE_IMGUI_USER_H" },

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
			"src/native/ui",
			"src/native/external",
            "src/native/external/imgui",
            "src/native/external/bx/include",
            "src/native/external/bgfx/include",
        	"api/include",
            "src/native/external/stb",
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
        
        -- autogenerated files
        ApiGen { Source = "src/tools/api_gen/pd_ui.json", Lang="-cpp", OutName = "api/include/pd_ui_autogen.h" },
        ApiGen { Source = "src/tools/api_gen/pd_ui.json", Lang="-rs",  OutName = "api/rust/prodbg/src/ui_ffi_autogen.rs" },
    },

	IdeGenerationHints = { Msvc = { SolutionFolder = "Libs" } },
}

-- vim: ts=4:sw=4:sts=4

