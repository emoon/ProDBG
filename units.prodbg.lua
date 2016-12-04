require "tundra.syntax.glob"
require "tundra.syntax.qt"
require "tundra.syntax.rust-cargo"
require "tundra.path"
require "tundra.util"

local native = require('tundra.native')

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
            Extensions = { ".cpp", ".h" },
            Recursive = true,
        },

        gen_uic("src/prodbg/Config/AmigaUAEConfig.ui"),
        gen_moc("src/prodbg/Config/AmigaUAEConfig.h"),

        gen_uic("src/prodbg/RegisterView/RegisterView.ui"),
        gen_moc("src/prodbg/RegisterView/RegisterView.h"),

        gen_uic("src/prodbg/mainwindow.ui"),
        gen_uic("src/prodbg/MemoryView/MemoryView.ui"),
        gen_moc("src/prodbg/MainWindow.h"),

        gen_moc("src/prodbg/Backend/IBackendRequests.h"),
        gen_moc("src/prodbg/Backend/BackendRequests.h"),
        gen_moc("src/prodbg/Backend/BackendHandler.h"),

        gen_moc("src/prodbg/CodeView/CodeView.h"),
        gen_moc("src/prodbg/MemoryView/MemoryView.h"),
        gen_moc("src/prodbg/MemoryView/MemoryViewWidget.h"),
    },

    Env = {
       CXXOPTS = {
            { "-isystem $(QT5)/lib/QtWidgets.framework/Headers",
              "-isystem $(QT5)/lib/QtCore.framework/Headers",
              "-isystem $(QT5)/lib/QtGui.framework/Headers",
              "-F$(QT5)/lib"; Config = "macosx-*-*" },

            { "-isystem $(QT5)/include/QtWidgets",
              "-isystem $(QT5)/include/QtCore",
              "-isystem $(QT5)/include/QtGui",
              "-isystem $(QT5)/include"; Config = "linux-*-*" },
        },

        CPPDEFS = {
            "QT_NO_KEYWORDS",
            "QT_NO_CAST_FROM_ASCII",
            "QT_NO_CAST_TO_ASCII",
        },

        CPPPATH = {
            "src/prodbg",
        	"api/include",
            "$(OBJECTROOT)", "$(OBJECTDIR)",
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

