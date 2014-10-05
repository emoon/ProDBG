
-----------------------------------------------------------------------------------------------------------------------

local mac_opts = {
	"-Wall",
	"-I.", "-DPRODBG_MAC", 
	"-Weverything", "-Werror", 
	"-Wno-c11-extensions",
	"-Wno-variadic-macros",
	"-Wno-c++98-compat-pedantic",
	"-Wno-old-style-cast",
	"-Wno-documentation", 
	"-Wno-missing-prototypes", 
	"-Wno-gnu-anonymous-struct",
	"-Wno-nested-anon-types",
	"-Wno-padded",
	"-DOBJECT_DIR=\\\"$(OBJECTDIR)\\\"",
	{ "-O0", "-g"; Config = "*-*-debug" },
	{ "-O3", "-g"; Config = "*-*-release" },
}

-----------------------------------------------------------------------------------------------------------------------

local macosx = {
    Env = {
        CCOPTS = mac_opts,
        CXXOPTS = mac_opts,
    },

    Frameworks = { "Cocoa" },
}

-----------------------------------------------------------------------------------------------------------------------

local win64 = {
    Env = {
        GENERATE_PDB = "1",
        CCOPTS = {
			"/DPRODBG_WIN",
            "/FS", "/MT", "/W4", "/I.", "/WX", "/DUNICODE", "/D_UNICODE", "/DWIN32", "/D_CRT_SECURE_NO_WARNINGS", "/wd4152", "/wd4996", "/wd4389", 
            { "/Od"; Config = "*-*-debug" },
            { "/O2"; Config = "*-*-release" },
        },

        CXXOPTS = {
			"/DPRODBG_WIN",
            "/FS", "/MT", "/I.", "/DUNICODE", "/D_UNICODE", "/DWIN32", "/D_CRT_SECURE_NO_WARNINGS", "/wd4152", "/wd4996", "/wd4389",
			"\"/DOBJECT_DIR=$(OBJECTDIR:#)\"",
            { "/Od"; Config = "*-*-debug" },
            { "/O2"; Config = "*-*-release" },
        },

		OBJCCOM = "meh",
    },
}

-----------------------------------------------------------------------------------------------------------------------

Build {

    Passes = {
        GenerateSources = { Name="Generate sources", BuildOrder = 1 },
    },

    Units = { 
    	"units.libs.lua",
    	"units.misc.lua",
    	"units.plugins.lua",
    	"units.prodbg.lua",
	},

    Configs = {
        Config { Name = "macosx-clang", DefaultOnHost = "macosx", Inherit = macosx, Tools = { "clang-osx" } },
        Config { Name = "win64-msvc", DefaultOnHost = { "windows" }, Inherit = win64, Tools = { "msvc" } },
    },

    IdeGenerationHints = {
        Msvc = {
            -- Remap config names to MSVC platform names (affects things like header scanning & debugging)
            PlatformMappings = {
                ['win64-msvc'] = 'x64',
            },
            -- Remap variant names to MSVC friendly names
            VariantMappings = {
                ['release']    = 'Release',
                ['debug']      = 'Debug',
            },
        },

		MsvcSolutions = {
			['ProDBG.sln'] = { } -- will get everything
		},
            
    },
    
}
