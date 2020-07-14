module(..., package.seeall)

local pkgconfig = require "tundra.syntax.pkgconfig"

function Configure(name, ctor)
  return pkgconfig.ConfigureRaw("llvm-config --cflags --libs", name, ctor)
end
