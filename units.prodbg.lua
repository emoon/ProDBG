require "tundra.syntax.glob"
require "tundra.syntax.qt"
require "tundra.syntax.rust-cargo"
require "tundra.path"
require "tundra.util"

local native = require('tundra.native')
local scanner = require "tundra.scanner"
local path = require "tundra.path"

DefRule {
  Name = "RccName",
  Command = "",

  Blueprint = {
    Source = { Required = true, Type = "string" },
  },

  Setup = function (env, data)
    local src = data.Source
    local base_name = path.get_filename_base(src)
    local pfx = 'qrc_'
    local ext = '.cpp'
    local out_name = "$(OBJECTDIR)$(SEP)" .. pfx .. base_name .. ext
    return {
      InputFiles = { src },
      OutputFiles = { out_name },
      Command = "$(QTRCCCMD) " .. src .. " -name " .. base_name .. " -o " .. out_name,
      Scanner = scanner.make_generic_scanner {
        Paths = { "." },
        KeywordsNoFollow = { "<file" },
        UseSeparators = true
      },
    }
  end,
}

local function gen_rcc(src)
    return RccName {
        Pass = "GenerateSources",
        Source = src
    }
end

local function gen_uic(src)
    return Uic {
        Pass = "GenerateSources",
        Source = src
    }
end


local function gen_moc(src)
    return Moc {
        Pass = "GenerateSources",
        Source = src
    }
end

-----------------------------------------------------------------------------------------------------------------------

Program {
    Name = "prodbg",
    Sources = {
        Glob {
            Dir = "src/prodbg",
            Extensions = { ".c",".cpp", ".h" },
            Recursive = true,
        },

        -- Themes
        gen_rcc("src/prodbg/themes/lightstyle/light.qrc"),
        gen_rcc("src/prodbg/themes/midnight/midnight.qrc"),
        gen_rcc("src/prodbg/themes/native/native.qrc"),
        gen_rcc("src/prodbg/themes/qdarkstyle/dark.qrc"),

        -- Dialogs
        gen_uic("src/prodbg/dialogs/PrefsDialog.ui"),
        gen_moc("src/prodbg/dialogs/PrefsDialog.h"),

        --

        gen_uic("src/prodbg/Config/AmigaUAEConfig.ui"),
        gen_moc("src/prodbg/Config/AmigaUAEConfig.h"),
        gen_moc("src/prodbg/Config/Config.h"),

        -- gen_uic("src/prodbg/RegisterView/RegisterView.ui"),
        -- gen_moc("src/prodbg/RegisterView/RegisterView.h"),

        gen_uic("src/prodbg/MainWindow.ui"),
        gen_uic("src/prodbg/MemoryView/MemoryView.ui"),
        gen_moc("src/prodbg/MainWindow.h"),

        gen_moc("src/prodbg/Backend/IBackendRequests.h"),
        gen_moc("src/prodbg/Backend/BackendRequests.h"),
        gen_moc("src/prodbg/Backend/BackendSession.h"),
        gen_moc("src/prodbg/AmigaUAE/AmigaUAE.h"),
        -- gen_moc("src/prodbg/PluginUI/signal_wrappers.h"),

        gen_moc("src/prodbg/View.h"),
        gen_moc("src/prodbg/ViewHandler.h"),
        gen_moc("src/prodbg/CodeView/CodeView.h"),
        gen_moc("src/prodbg/CodeView/DisassemblyView.h"),
        gen_moc("src/prodbg/MemoryView/MemoryView.h"),
        gen_moc("src/prodbg/MemoryView/MemoryViewWidget.h"),

        gen_moc("src/prodbg/PluginUI/generated/qt_api_gen.h"),
    },

    Env = {
       CXXOPTS = {
            { "-isystem $(QT5_LIB)/QtWidgets.framework/Headers",
              "-isystem $(QT5_LIB)/QtCore.framework/Headers",
              "-isystem $(QT5_LIB)/QtGui.framework/Headers",
              "-F$(QT5_LIB)/lib"; Config = "macosx-*-*" },
            --{ "-isystem $(QT5_INC)"; Config = "linux-*-*" },
        },

        CPPDEFS = {
            "QT_NO_KEYWORDS",
            "QT_NO_CAST_FROM_ASCII",
            "QT_NO_CAST_TO_ASCII",
        },

        CPPPATH = {
            "$(QT5_INC)",
            "src/native/external/tinyexpr",
            "src/prodbg",
        	"api/include",
        	"src/native/external",
        	"src/native/external/toolwindowmanager",
            "$(QT5_INC)",
            "$(QT5_INC)/QtCore",
            "$(QT5_INC)/QtWidgets",
            "$(QT5_INC)/QtGui",
            "$(OBJECTROOT)", "$(OBJECTDIR)",
        },

        LIBPATH = {
			{ "$(QT5_LIB)"; Config = "win64-*-*" },
			{ "$(QT5_LIB)"; Config = "linux-*-*" },
		},

        PROGCOM = {
            {  "-Wl,-rpath,$(QT5_LIB)", "-F$(QT5_LIB)", "-lstdc++", Config = "macosx-clang-*" },
            {  "-Wl,-rpath,$(QT5_LIB)", "-lstdc++", "-lm", "-ldl"; Config = "linux-*-*" },
        },
    },

	Libs = {
		{ "wsock32.lib", "kernel32.lib", "user32.lib", "gdi32.lib", "Comdlg32.lib",
		  "Advapi32.lib", "Qt5Gui.lib", "Qt5Core.lib", "Qt5Widgets.lib"; Config = "win64-*-*" },
		{ "Qt5Gui", "Qt5Core", "Qt5Widgets"; Config = "linux-*-*" },
	},

    Frameworks = { "Cocoa", "QtWidgets", "QtGui", "QtCore" },

    Depends = { "remote_api", "capstone", "tinyexpr", "toolwindowmanager" },
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

