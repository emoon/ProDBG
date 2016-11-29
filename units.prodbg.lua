require "tundra.syntax.glob"
require "tundra.syntax.rust-cargo"
require "tundra.path"
require "tundra.util"

local native = require('tundra.native')

-----------------------------------------------------------------------------------------------------------------------
-- Used to generate the moc cpp files as needed for .h that uses Q_OBJECT

DefRule {
	Name = "MocGeneration",
	Pass = "GenerateSources",
	Command = "$(QT5)/bin/moc --no-notes $(<) -o $(@)",

	Blueprint = {
		Source = { Required = true, Type = "string", Help = "Input filename", },
		OutName = { Required = true, Type = "string", Help = "Output filename", },
	},

	Setup = function (env, data)
		return {
			InputFiles    = { data.Source },
			OutputFiles   = { "$(OBJECTDIR)/_generated/" .. data.OutName },
		}
	end,
}

-- Used to send a list of header files

local function MocGenerationMulti(sources)
 local result = {}
 for _, src in ipairs(tundra.util.flatten(sources)) do
   result[#result + 1] = MocGeneration { Source = src, OutName = tundra.path.get_filename_base(src) .. "_moc.cpp" }
 end
 return result
end

-----------------------------------------------------------------------------------------------------------------------

local function get_rs_src(dir)
	return Glob {
		Dir = dir,
		Extensions = { ".rs" },
		Recursive = true,
	}
end

-----------------------------------------------------------------------------------------------------------------------

StaticLibrary {
    Name = "qt_main",

    Env = {
        CXXOPTS = {
			{
              "-isystem $(QT5)/lib/QtWidgets.framework/Headers",
              "-isystem $(QT5)/lib/QtCore.framework/Headers",
              "-F$(QT5)/lib"; Config = "macosx-*-*" },
        },

        FRAMEWORKS = { "$(QT5)/lib/QtCore", "$(QT5)/lib/QtWidgets" },
    },

    Sources = {
		MocGenerationMulti {
			Glob {
				Dir = "src/qt_main",
				Extensions = { ".h" }
			},
		},

		Glob {
			Dir = "src/qt_main",
			Extensions = { ".cpp" }
		},
    },
}

-----------------------------------------------------------------------------------------------------------------------

RustCrate {
	Name = "prodbg_api",
	CargoConfig = "api/rust/prodbg/Cargo.toml",
	Sources = {
		get_rs_src("api/rust/prodbg"),
	},
}

-----------------------------------------------------------------------------------------------------------------------

RustProgram {
	Name = "prodbg",
	CargoConfig = "src/prodbg/main/Cargo.toml",
	Sources = {
		get_rs_src("src/prodbg/main"),
		"src/prodbg/build.rs",
	},

    Depends = { "remote_api",
    			"capstone",
    			"qt_main",
    			"prodbg_api" },
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

if native.host_platform == "macosx" then
	Default "prodbg"
	Default(prodbgBundle)
else
	Default "prodbg"
end

-- vim: ts=4:sw=4:sts=4

