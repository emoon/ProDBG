-- Microsoft Visual Studio 2019 Solution/Project file generation

module(..., package.seeall)

local msvc_common = require "tundra.ide.msvc-common"

msvc_common.setup("12.00", "2019")
