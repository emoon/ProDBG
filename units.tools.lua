require "tundra.syntax.glob"
require "tundra.path"
require "tundra.util"

local BX_DIR = "src/external/bx/"
local BGFX_DIR = "src/external/bgfx/"
local GLSL_OPTIMIZER = BGFX_DIR  .. "3rdparty/glsl-optimizer/"
local FCPP_DIR = BGFX_DIR .. "3rdparty/fcpp/"

-----------------------------------------------------------------------------------------------------------------------

Program {
	Name = "bgfx_shaderc",
	Target = "$(BGFX_SHADERC)";

    Env = {
        CCOPTS = {
        	{ "-Wno-everything"; Config = "macosx-*-*" },
        },

        CXXOPTS = {
        	{ "-Wno-everything"; Config = "macosx-*-*" },
        },

		CPPPATH = { 
			{ 
			  GLSL_OPTIMIZER .. "src/glsl/msvc",
			  GLSL_OPTIMIZER .. "include/c99";
			  "$(DXSDK_DIR)/include" ; Config = "win64-*-*" },
			{
				BX_DIR   .. "include",
				BGFX_DIR .. "include",
				FCPP_DIR,
				GLSL_OPTIMIZER .. "src",
				GLSL_OPTIMIZER .. "include",
				GLSL_OPTIMIZER .. "src/mesa",
				GLSL_OPTIMIZER .. "src/mapi",
				GLSL_OPTIMIZER .. "src/glsl",
			},
		},

		CPPDEFS = {
			{ "__STDC__", "__STDC_VERSION__=199901L", "strdup=_strdup", "alloca=_alloca", "isascii=__isascii"; Config = "win64-*-*" },
			{ "NINCLUDE=64", "NWORK=65536", "NBUFF=65536", "OLD_PREPROCESSOR=0" },
		},

		PROGCOM = {
			{ "-lstdc++"; Config = { "macosx-clang-*", "linux-gcc-*" } },
			{ "-lm -lpthread -ldl -lX11"; Config = "linux-*-*" },
		},
	},

	Sources = {
		BGFX_DIR .. "tools/shaderc/shaderc.cpp",
		BGFX_DIR .. "src/vertexdecl.cpp",
		BGFX_DIR .. "src/vertexdecl.h",

		FCPP_DIR .. "cpp1.c",
		FCPP_DIR .. "cpp2.c",
		FCPP_DIR .. "cpp3.c",
		FCPP_DIR .. "cpp4.c",
		FCPP_DIR .. "cpp5.c",
		FCPP_DIR .. "cpp6.c",

		Glob { Dir = GLSL_OPTIMIZER .. "src/mesa", Extensions = { ".c", ".h" } },
		Glob { Dir = GLSL_OPTIMIZER .. "src/glsl", Extensions = { ".cpp", ".c", ".h" } },
		Glob { Dir = GLSL_OPTIMIZER .. "src/util", Extensions = { ".c", ".h" } },
	},

    Libs = { { "kernel32.lib", "d3dx9.lib", "d3dcompiler.lib", "dxguid.lib" ; Config = "win64-*-*" } },

    Frameworks = { "Cocoa"  },
}

Default "bgfx_shaderc"

