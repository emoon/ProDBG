require "tundra.syntax.glob"
require "tundra.syntax.qt"
require "tundra.syntax.rust-cargo"
require "tundra.path"
require "tundra.util"

local native = require('tundra.native')

-----------------------------------------------------------------------------------------------------------------------

Program {
	Name = "prodbg",
	Sources = {
        Glob {
            Dir = "src/prodbg",
            Extensions = { ".cpp", ".h" },
            Recursive = true,
        },

        Moc { Source = "src/prodbg/MainWindow.h" },
        Moc { Source = "src/prodbg/CodeView/CodeView.h" },
        Moc { Source = "src/prodbg/MemoryView/MemoryViewInternal.h" },
        Moc { Source = "src/prodbg/MemoryView/MemoryViewWidget.h" },
	},

    Env = {
       CXXOPTS = {
            { "-isystem $(QT5)/lib/QtWidgets.framework/Headers",
              "-isystem $(QT5)/lib/QtCore.framework/Headers",
              "-isystem $(QT5)/lib/QtGui.framework/Headers",
              "-F$(QT5)/lib"; Config = "macosx-*-*" },
        },

        PROGCOM = {
            {  "-Wl,-rpath,$(QT5)/lib", "-F$(QT5)/lib", "-lstdc++", Config = "macosx-clang-*" },
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

