
module(..., package.seeall)

local vscommon = require "tundra.tools.msvc-vscommon-next"

function apply(env, options)
  vscommon.apply_msvc_visual_studio("2017", env, options)
end
