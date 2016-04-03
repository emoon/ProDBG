require "tundra.syntax.glob"
require "tundra.syntax.rust-cargo"
require "tundra.path"
require "tundra.util"

local native = require('tundra.native')

-----------------------------------------------------------------------------------------------------------------------

local function get_rs_src(dir)
	return Glob {
		Dir = dir,
		Extensions = { ".rs" },
		Recursive = true,
	}
end

-----------------------------------------------------------------------------------------------------------------------

local function get_rs_native_src(dir)
	return Glob {
		Dir = dir,
		Extensions = { ".rs", ".c", ".m", ".mm" },
		Recursive = true,
	}
end

-----------------------------------------------------------------------------------------------------------------------

RustCrate {
	Name = "bgfx-rs",
	CargoConfig = "src/prodbg/bgfx/Cargo.toml",
	Sources = {
		get_rs_src("src/prodbg/bgfx"),
		get_rs_src("api/rust"),
		"src/prodbg/build.rs",
	},
}

-----------------------------------------------------------------------------------------------------------------------

RustCrate {
	Name = "imgui-sys",
	CargoConfig = "src/prodbg/imgui_sys/Cargo.toml",
	Sources = {
		get_rs_src("src/prodbg/imgui_sys"),
		get_rs_src("api/rust"),
		"src/prodbg/build.rs",
	},
}

-----------------------------------------------------------------------------------------------------------------------

RustCrate  {
	Name = "core",
	CargoConfig = "src/prodbg/core/Cargo.toml",
	Sources = {
		get_rs_src("src/prodbg/core"),
		"src/prodbg/build.rs",
	},
}

-----------------------------------------------------------------------------------------------------------------------

RustProgram {
	Name = "ui_testbench",
	CargoConfig = "src/prodbg/ui_testbench/Cargo.toml",
	Sources = {
		get_rs_src("src/prodbg/ui_testbench"),
		-- get_rs_src("src/prodbg/core"),
		"src/prodbg/build.rs",
	},

    Depends = { "ui", "lua", "remote_api", "stb", "bgfx", "bgfx-rs",
    			"imgui", "scintilla", "tinyxml2", "capstone", "imgui-sys", "core" },
}

-----------------------------------------------------------------------------------------------------------------------

RustProgram {
	Name = "prodbg",
	CargoConfig = "src/prodbg/main/Cargo.toml",
	Sources = {
		get_rs_src("src/prodbg/main"),
		-- get_rs_src("src/prodbg/core"),
		get_rs_src("src/ui"),
		"src/prodbg/build.rs",
	},

    Depends = { "ui", "lua", "remote_api", "stb", "bgfx", "bgfx-rs",
    			"imgui", "scintilla", "tinyxml2", "capstone", "imgui-sys", "core" },
}

-----------------------------------------------------------------------------------------------------------------------

local prodbgBundle = OsxBundle
{
	Depends = { "prodbg" },
	Target = "$(OBJECTDIR)/ProDBG.app",
	InfoPList = "Data/Mac/Info.plist",
	Executable = "$(OBJECTDIR)/prodbg",
	Resources = {
		CompileNib { Source = "data/mac/appnib.xib", Target = "appnib.nib" },
		"data/mac/icon.icns",
	},

	Config = { "macosx-clang-debug-default" ; "macosx-clang-release-default" },
}

-----------------------------------------------------------------------------------------------------------------------

local uiBundle = OsxBundle
{
	Depends = { "ui_testbench" },
	Target = "$(OBJECTDIR)/UITestbench.app",
	InfoPList = "Data/Mac/Info.plist",
	Executable = "$(OBJECTDIR)/ui_testbench",
	Resources = {
		CompileNib { Source = "data/mac/appnib.xib", Target = "appnib2.nib" },
		"data/mac/icon.icns",
	},

	Config = { "macosx-clang-debug-default" ; "macosx-clang-release-default" },
}

-----------------------------------------------------------------------------------------------------------------------

if native.host_platform == "macosx" then
	Default(prodbgBundle)
	Default(uiBundle)
else
	Default "prodbg"
	Default "ui_testbench"
end

