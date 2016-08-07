require "tundra.syntax.glob"
require "tundra.syntax.rust-cargo"
require "tundra.path"
require "tundra.util"

local native = require('tundra.native')

-----------------------------------------------------------------------------------------------------------------------

local function get_rs_src(dir)
	return Glob {
		Dir = dir,
		Extensions = { ".rs", ".in" },
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
	Name = "prodbg_api",
	CargoConfig = "api/rust/prodbg/Cargo.toml",
	Sources = {
		get_rs_src("api/rust/prodbg"),
	},
}

-----------------------------------------------------------------------------------------------------------------------

RustCrate {
	Name = "viewdock",
	CargoConfig = "src/prodbg/viewdock/Cargo.toml",
	Sources = {
		get_rs_src("src/prodbg/viewdock"),
	},
}

-----------------------------------------------------------------------------------------------------------------------

RustCrate {
	Name = "settings",
	CargoConfig = "src/crates/settings/Cargo.toml",
	Sources = {
		get_rs_src("src/crates/settings"),
	},
}

-----------------------------------------------------------------------------------------------------------------------

RustCrate {
	Name = "project",
	CargoConfig = "src/crates/project/Cargo.toml",
	Sources = {
		get_rs_src("src/crates/project"),
	},
}

-----------------------------------------------------------------------------------------------------------------------

RustCrate {
	Name = "bgfx",
	CargoConfig = "src/prodbg/bgfx-rs/Cargo.toml",
	Sources = {
		get_rs_src("src/prodbg/bgfx-rs"),
		get_rs_src("api/rust"),
		"src/prodbg/build.rs",
	},

    Depends = { "bgfx_native" },
}

-----------------------------------------------------------------------------------------------------------------------

RustCrate {
	Name = "renderer",
	CargoConfig = "src/prodbg/renderer/Cargo.toml",
	Sources = {
		get_rs_src("src/prodbg/renderer"),
		get_rs_src("src/prodbg/imgui_sys"),
		get_rs_src("src/prodbg/bgfx-rs"),
		"src/prodbg/build.rs",
	},
}


-----------------------------------------------------------------------------------------------------------------------

RustCrate {
	Name = "imgui_sys",
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

RustCrate  {
	Name = "serde_macros",
	CargoConfig = "src/helpers/serde_macros/Cargo.toml",
	Sources = {
		get_rs_src("src/helpers/serde_macros"),
	},
}

-----------------------------------------------------------------------------------------------------------------------

--[[

RustProgram {
	Name = "ui_testbench",
	CargoConfig = "src/prodbg/ui_testbench/Cargo.toml",
	Sources = {
		get_rs_src("src/prodbg/ui_testbench"),
		"src/prodbg/build.rs",
	},

    Depends = { "lua", "remote_api", "stb", "bgfx_native", "bgfx", "ui",
    			"imgui", "scintilla", "tinyxml2", "capstone", "imgui_sys", "core" },
}

--]]

-----------------------------------------------------------------------------------------------------------------------

RustProgram {
	Name = "prodbg",
	CargoConfig = "src/prodbg/main/Cargo.toml",
	Sources = {
		get_rs_src("src/prodbg/main"),
		get_rs_src("src/helpers/serde_macros"),
		-- get_rs_src("src/prodbg/core"),
		-- get_rs_src("src/ui"),
		"src/prodbg/build.rs",
	},

    Depends = { "lua", "remote_api", "stb", "bgfx_native", "bgfx", "ui",
    			"imgui", "tinyxml2", "capstone", "renderer", "scintilla",
    			"imgui_sys", "core", "viewdock", "settings", "prodbg_api",
    			"settings", "project", "serde_macros" },
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

--[[

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

--]]

-----------------------------------------------------------------------------------------------------------------------

if native.host_platform == "macosx" then
	Default "prodbg"
	--Default "ui_testbench"
	Default(prodbgBundle)
	--Default(uiBundle)
else
	Default "prodbg"
	--Default "ui_testbench"
end

-- vim: ts=4:sw=4:sts=4

