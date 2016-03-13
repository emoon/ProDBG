
-----------------------------------------------------------------------------------------------------------------------

local mac_opts = {
	"-Wall",
	"-Wno-switch-enum",
	"-I.", "-DPRODBG_MAC", 
	"-Weverything", "-Werror", 
	"-Wno-unknown-warning-option",
	"-Wno-c11-extensions",
	"-Wno-variadic-macros",
	"-Wno-c++98-compat-pedantic",
	"-Wno-old-style-cast",
	"-Wno-documentation", 
	"-Wno-reserved-id-macro",
	"-Wno-missing-prototypes", 
	"-Wno-deprecated-declarations",
	"-Wno-cast-qual",
	"-Wno-gnu-anonymous-struct",
	"-Wno-nested-anon-types",
	"-Wno-padded",
	"-Wno-c99-extensions",
	"-Wno-missing-field-initializers",
	"-Wno-weak-vtables",
	"-Wno-format-nonliteral",
	"-Wno-non-virtual-dtor",
	"-DOBJECT_DIR=\\\"$(OBJECTDIR)\\\"",
	{ "-O0", "-g"; Config = "*-*-debug" },
	{ "-O3", "-g"; Config = "*-*-release" },
}

local macosx = {
    Env = {
    	RUST_CARGO_OPTS = { 
			{ "test"; Config = "*-*-*-test" },
		},

        CCOPTS =  {
			mac_opts,
		},

        CXXOPTS = {
			mac_opts,
			"-std=c++11",
		},

        SHLIBOPTS = { "-lstdc++" },
		PROGCOM = { "-lstdc++" },

        BGFX_SHADERC = "$(OBJECTDIR)$(SEP)bgfx_shaderc$(PROGSUFFIX)",
    },

    Frameworks = { "Cocoa" },
}

local macosx_test = {
    Env = {
        CCOPTS =  {
			mac_opts,
			"-Wno-everything",
			"-coverage",
		},

        CXXOPTS = {
			mac_opts,
			"-Wno-everything",
			"-coverage",
			"-std=c++11",
		},

        SHLIBOPTS = { "-lstdc++", "-coverage" },
		PROGCOM = { "-lstdc++", "-coverage" },

        BGFX_SHADERC = "$(OBJECTDIR)$(SEP)bgfx_shaderc$(PROGSUFFIX)",
    },

    Frameworks = { "Cocoa" },
}

-----------------------------------------------------------------------------------------------------------------------
        
local gcc_opts = {
	"-I.",
	"-Wno-array-bounds", "-Wno-attributes", "-Wno-unused-value",
	"-DOBJECT_DIR=\\\"$(OBJECTDIR)\\\"",
	"-Wall", "-DPRODBG_UNIX",
	"-fPIC",
	{ "-O0", "-g"; Config = "*-*-debug" },
	{ "-O3", "-g"; Config = "*-*-release" },
}

local gcc_env = {
    Env = {
    	RUST_CARGO_OPTS = { 
			{ "test"; Config = "*-*-*-test" },
		},

        CCOPTS = {
			gcc_opts,
		},

        CXXOPTS = {
			gcc_opts,
			"-std=c++11",
		},

        BGFX_SHADERC = "$(OBJECTDIR)$(SEP)bgfx_shaderc$(PROGSUFFIX)",
    },

	ReplaceEnv = {
		PROGCOM = "$(LD) $(PROGOPTS) $(LIBPATH:p-L) -o $(@) -Wl,--start-group $(LIBS:p-l) $(<) -Wl,--end-group"
	},
}

-----------------------------------------------------------------------------------------------------------------------

local win64_opts = {
	"/DPRODBG_WIN",
	"/EHsc", "/FS", "/MT", "/W3", "/I.", "/WX", "/DUNICODE", "/D_UNICODE", "/DWIN32", "/D_CRT_SECURE_NO_WARNINGS", "/wd4200", "/wd4152", "/wd4996", "/wd4389", "/wd4201", "/wd4152", "/wd4996", "/wd4389",
	"\"/DOBJECT_DIR=$(OBJECTDIR:#)\"",
	{ "/Od"; Config = "*-*-debug" },
	{ "/O2"; Config = "*-*-release" },
}

local win64 = {
    Env = {
    	RUST_CARGO_OPTS = { 
			{ "test"; Config = "*-*-*-test" },
		},

        GENERATE_PDB = "1",
        CCOPTS = {
			win64_opts,
        },

        CXXOPTS = {
			win64_opts,
        },

        BGFX_SHADERC = "$(OBJECTDIR)$(SEP)bgfx_shaderc$(PROGSUFFIX)",
		OBJCCOM = "meh",
    },
}

-----------------------------------------------------------------------------------------------------------------------

Build {

    Passes = {
        BuildTools = { Name="Build Tools", BuildOrder = 1 },
        GenerateSources = { Name="Generate sources", BuildOrder = 2 },
    },

    Units = { 
    	"units.tools.lua",
    	"units.libs.lua",
    	"units.misc.lua",
    	"units.plugins.lua",
    	"units.prodbg.lua",
    	-- "units.tests.lua",
	},

    Configs = {
        Config { Name = "macosx-clang", DefaultOnHost = "macosx", Inherit = macosx, Tools = { "clang-osx", "rust" } },
        Config { Name = "macosx_test-clang", SupportedHosts = { "macosx" }, Inherit = macosx_test, Tools = { "clang-osx", "rust" } },
        Config { Name = "win64-msvc", DefaultOnHost = { "windows" }, Inherit = win64, Tools = { { "msvc", "rust" }, "generic-asm" } },
        Config { Name = "linux-gcc", DefaultOnHost = { "linux" }, Inherit = gcc_env, Tools = { "gcc", "rust" } },
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
	
	Variants = { "debug", "release" },
	SubVariants = { "default", "test" },
}
