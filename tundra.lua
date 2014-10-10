
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
	"-std=c++11",
	"-DOBJECT_DIR=\\\"$(OBJECTDIR)\\\"",
	{ "-O0", "-g"; Config = "*-*-debug" },
	{ "-O3", "-g"; Config = "*-*-release" },
}

local macosx = {
    Env = {
        CCOPTS =  {
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
		},
        
        CXXOPTS = {
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
			"-std=c++11",
			"-DOBJECT_DIR=\\\"$(OBJECTDIR)\\\"",
			{ "-O0", "-g"; Config = "*-*-debug" },
			{ "-O3", "-g"; Config = "*-*-release" },
		},
    },

    Frameworks = { "Cocoa" },
}

-----------------------------------------------------------------------------------------------------------------------

local gcc_opts = {
	"-I.",
	"-Wno-unused-value",
	"-DOBJECT_DIR=\\\"$(OBJECTDIR)\\\"",
	"-Wall", "-DPRODBG_UNIX", "-std=gnu99",
	{ "-O0", "-g"; Config = "*-*-debug" },
	{ "-O3", "-g"; Config = "*-*-release" },
}

local gcc_env = {
    Env = {
        CCOPTS = gcc_opts,
        CXXOPTS = gcc_opts,
    },

	ReplaceEnv = {
		PROGCOM = "$(LD) $(PROGOPTS) $(LIBPATH:p-L) -o $(@) -Wl,--start-group $(LIBS:p-l) $(<) -Wl,--end-group"
	},
}

-----------------------------------------------------------------------------------------------------------------------

local win64 = {
    Env = {
        GENERATE_PDB = "1",
        CCOPTS = {
			"/DPRODBG_WIN",
            "/FS", "/MT", "/W4", "/I.", "/WX", "/DUNICODE", "/D_UNICODE", "/DWIN32", "/D_CRT_SECURE_NO_WARNINGS", "/wd4152", "/wd4996", "/wd4389", "/wd4201", 
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
        Config { Name = "linux-gcc", DefaultOnHost = { "linux" }, Inherit = gcc_env, Tools = { "gcc" } },
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
