-- Utility script to generate a build file from a set of source files.

module(..., package.seeall)

require "strict"

local template = [===[

require "tundra.syntax.glob"

local common = {
 Env = {
   -- Global, shared environment settings can go here
 },
 Defines = {
   { "_DEBUG"; Config = '*-*-debug' },
   { "NDEBUG"; Config = '*-*-release' },
 },
}

local win64 = {
  Inherit = common,
  Env = {
      GENERATE_PDB = "1",
      CCOPTS = {
        "/FS",
        "/W4",
        { "/Od"; Config = "*-*-debug" },
        { "/O2"; Config = "*-*-production" },
        { "/Ox"; Config = "*-*-release" },
      },
      CXXOPTS = {
        "$(CCOPTS)",
        "/EHsc",
      },
  },
}

local macosx = {
  Inherit = common,
  Env = {
    CCOPTS = {
			{ "-O0", "-g"; Config = "*-*-debug" },
			{ "-O2", "-g"; Config = "*-*-production" },
			{ "-O3"; Config = "*-*-release" },
    },
    CXXOPTS = {
      "$(CCOPTS)",
			"-std=c++14",
    },
  },
	ReplaceEnv = {
		["LD"] = "$(CXX)",
	},
}

local linux = {
  Inherit = common,
  Env = {
    CCOPTS = {
			{ "-O0", "-g"; Config = "*-*-debug" },
			{ "-O2", "-g"; Config = "*-*-production" },
			{ "-O3"; Config = "*-*-release" },
    },
    CXXOPTS = {
      "$(CCOPTS)",
			"-std=c++14",
    },
  },
	ReplaceEnv = {
		["LD"] = "$(CXX)",
	},
}

Build {
  Passes = {
    -- Define any additional passes you need here for reliable code generation/include interactions.
    -- Foo = { Name = "Foo", BuildOrder = 1 },
  },
  Configs = {
    Config {
      Name = "win64-mscv",
      SupportedHosts = { "windows" },
      DefaultOnHost = "windows",
      Tools = { "msvc" },
      Inherit = win64,
    },
    Config {
      Name = "macosx-clang",
      SupportedHosts = { "macosx" },
      DefaultOnHost = "macosx",
      Tools = { "clang-osx" },
      Inherit = macosx,
    },
    Config {
      Name = "linux-gcc",
      SupportedHosts = { "linux" },
      DefaultOnHost = "linux",
      Tools = { "gcc" },
      Inherit = linux,
    },
  },

  Units = function()
    local prog = Program {
      Name = "program",
      Sources = {
        FGlob {
          Dir = ".",
          Extensions = { ".c", ".cpp" },
          Filters = {},
        },
      },
    }
    Default(prog)
  end,
}

]===]

function generate_template_file()
  print "Generating tundra.lua template.."
  local fh = assert(io.open("tundra.lua", "w"))
  fh:write(template)
  fh:close()
end


