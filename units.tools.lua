require "tundra.syntax.glob"
require "tundra.util"

local native = require('tundra.native')
local path = require('tundra.path')

local BX_DIR = "src/external/bx/"
local BGFX_DIR = "src/external/bgfx/"
local GLSL_OPTIMIZER = BGFX_DIR  .. "3rdparty/glsl-optimizer/"
local FCPP_DIR = BGFX_DIR .. "3rdparty/fcpp/"

-- setup target for shader

local shaderc_platform = "windows"

if native.host_platform == "macosx" then
	shaderc_platform = "osx"
else
	shaderc_platform = "linux"
end

-----------------------------------------------------------------------------------------------------------------------

DefRule {
	Name = "ShadercFS",
	Command = "$(BGFX_SHADERC) -f $(<) -o $(@) --type fragment --platform " .. shaderc_platform,

	Blueprint = {
		Source = { Required = true, Type = "string", Help = "Input filename", },
		OutName = { Required = false, Type = "string", Help = "Output filename", },
	},

	Setup = function (env, data)
		return {
			InputFiles    = { data.Source },
			OutputFiles   = { "$(OBJECTDIR)/_generated/" .. path.drop_suffix(data.Source) .. ".fs" },
		}
	end,
}

DefRule {
	Name = "ShadercVS",
	Command = "$(BGFX_SHADERC) -f $(<) -o $(@) --type vertex --platform " .. shaderc_platform,

	Blueprint = {
		Source = { Required = true, Type = "string", Help = "Input filename", },
		OutName = { Required = false, Type = "string", Help = "Output filename", },
	},

	Setup = function (env, data)
		return {
			InputFiles    = { data.Source },
			OutputFiles   = { "$(OBJECTDIR)/_generated/" .. path.drop_suffix(data.Source) .. ".vs" },
		}
	end,
}

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

