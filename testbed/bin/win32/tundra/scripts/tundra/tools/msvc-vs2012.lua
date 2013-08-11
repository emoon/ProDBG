-- msvc-vs2012.lua - Settings to use Microsoft Visual Studio 2012 from the
-- registry.

module(..., package.seeall)

local vscommon = require "tundra.tools.msvc-vscommon"

function apply(env, options)
  vscommon.apply_msvc_visual_studio("11.0", env, options)
end
