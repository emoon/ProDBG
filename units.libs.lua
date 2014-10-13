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
        	"-Wno-unused-variable",
        	"-Wno-pointer-to-int-cast",
        	"-Wno-return-type",
        	"-Wno-unused-function"
        	; Config = "linux-*-*" },
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
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "yaml",

    Env = { 
		CPPPATH = { 
			"src/external/libyaml/include",
		},

        CCOPTS = {
        	{ "-Wno-everything"; Config = "macosx-*-*" },
        	{ "/DYAML_DECLARE_STATIC", "/wd4267", "/wd4706", "/wd4244", "/wd4701", "/wd4334", "/wd4127", "/wd4245"; Config = "win64-*-*" },
        },
    },

    Sources = { 
        Glob {
            Dir = "src/external/libyaml/src",
            Extensions = { ".c", ".h" },
        },
    },
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
    		Recursive = false } ; Config = { "macosx-*-*", "linux-*-*" }
    	},

    	-- Mac

		{ "src/external/libuv/src/unix/darwin/darwin-proctitle.c",
		  "src/external/libuv/src/unix/darwin/darwin.c" ; Config = "macosx-*-*" },

		-- Linux

		{ "src/external/libuv/src/unix/linux/linux-core.c",
		  "src/external/libuv/src/unix/linux/linux-inotify.c",
		  "src/external/libuv/src/unix/linux/linux-syscalls.c" ; Config = "linux-*-*" },
	},
}


-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "bgfx",

    Env = { 
        CPPPATH = { 
            "src/external/bgfx/include",
            "src/external/bx/include",
            "src/external/bgfx/3rdparty/khronos",
        },
        
        CXXOPTS = {
			{ "-Wno-variadic-macros", "-Wno-everything" ; Config = "macosx-*-*" },
			{ "/EHsc"; Config = "win64-*-*" },
        },
    },

    Sources = { 
		{ "src/external/bgfx/src/bgfx.cpp",
		  "src/external/bgfx/src/image.cpp",
		  "src/external/bgfx/src/vertexdecl.cpp",
		  "src/external/bgfx/src/renderer_gl.cpp",
		  "src/external/bgfx/src/renderer_null.cpp",
		  "src/external/bgfx/src/renderer_d3d9.cpp", 
		  "src/external/bgfx/src/renderer_d3d11.cpp" }, 
	    { "src/external/bgfx/src/glcontext_wgl.cpp" ; Config = "win64-*-*" },
	    -- { "src/external/bgfx/src/glcontext_glx.cpp" ; Config = "linux-*-*" },
	    { "src/external/bgfx/src/glcontext_nsgl.mm" ; Config = "macosx-*-*" },
    },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "nanovg",

    Env = { 
        CPPPATH = { 
            "src/external/nanovg",
            "src/external/stb",
            "src/external/bgfx/include",
        },
        
        CXXOPTS = {
        	"-Wno-variadic-macros", 
        	"-Wno-everything" ; Config = "macosx-*-*" 
        },
    },

    Sources = { 
        Glob {
            Dir = "src/external/nanovg",
            Extensions = { ".cpp", ".h" },
        },
    },
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
}


-----------------------------------------------------------------------------------------------------------------------
----------------------------------------------- INTERNAL LIBS --------------------------------------------------------- 
-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "core",

    Env = { 
        CPPPATH = { 
            "src/external/stb",
			"src/external/libuv/include",
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
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "session",

    Env = { 
        CPPPATH = { 
            "src/external/stb",
			"src/external/libuv/include",
        	"api/include",
            "src/prodbg",
        },
    },

    Sources = { 
        Glob {
            Dir = "src/prodbg/session",
            Extensions = { ".cpp", ".h" },
        },
    },
}

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "ui",

    Env = { 

        CXXOPTS = {
        	{ "-Wno-gnu-anonymous-struct",
			  "-Wno-global-constructors",
			  "-Wno-switch-enum",
			  "-Wno-nested-anon-types",
			  "-Wno-float-equal",
			  "-Wno-cast-align",
			  "-Wno-exit-time-destructors",
			  "-Wno-format-nonliteral"; Config = "macosx-*-*" },
        },

        CPPPATH = { 
            "src/external/nanovg",
            "src/external/stb",
			"src/external/libyaml/include",
            "src/prodbg",
        },
    },

    Sources = { 
        Glob {
            Dir = "src/prodbg/ui",
            Extensions = { ".c", ".cpp", ".h" },
            Recursive = true,
        },
    },
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
}


