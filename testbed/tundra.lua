require "tundra.native"
local native = require('tundra.native')

local macosx = {
	Env = {
		QT5 = native.getenv("QT5"),
		CCOPTS = {
			"-Wall",
			"-Wno-deprecated-declarations", -- TickCount issue no Mountain Lion (needs to be fixed)
			"-I.", "-DMACOSX", "-Wall",
			{ "-O0", "-g"; Config = "*-*-debug" },
			{ "-O3"; Config = "*-*-release" },
		},
	},

	Frameworks = { "Cocoa" },
}

local win32 = {
	Env = {
		QT5 = native.getenv("QT5"),
 		GENERATE_PDB = "1",
		CCOPTS = {
			"/W4", "/I.", "/WX", "/DUNICODE", "/D_UNICODE", "/DWIN32", "/D_CRT_SECURE_NO_WARNINGS", "/wd4996", "/wd4389",
			{ "/Od"; Config = "*-*-debug" },
			{ "/O2"; Config = "*-*-release" },
		},
	},
}

Build {
	Units = "units.lua",

	Configs = {
		Config { Name = "macosx-clang", DefaultOnHost = "macosx", Inherit = macosx, Tools = { "clang-osx" } },
		Config { Name = "win32-msvc", DefaultOnHost = { "windows" }, Inherit = win32, Tools = { "msvc" } },
	},
}
