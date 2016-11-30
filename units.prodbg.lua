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

Program {
	Name = "prodbg",
	Sources = {
        Glob {
            Dir = "src/prodbg",
            Extensions = { ".cpp", ".h" },
            Recursive = true,
        },

		MocGenerationMulti {
			Glob {
				Dir = "src/prodbg",
				Extensions = { ".h" }
			},
		},
	},

    Env = {
       CXXOPTS = {
            { "-isystem $(QT5)/lib/QtWidgets.framework/Headers",
              "-isystem $(QT5)/lib/QtCore.framework/Headers",
              "-isystem $(QT5)/lib/QtGui.framework/Headers",
              "-F$(QT5)/lib"; Config = "macosx-*-*" },
        },

        PROGCOM = {
            { "-F$(QT5)/lib", "-lstdc++", Config = "macosx-clang-*" },
        },
    },

	Frameworks = { "Cocoa", "QtWidgets", "QtGui", "QtCore" },

    Depends = { "remote_api", "capstone" },
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

