-- lemon.lua - Support for the Lemon parser generator

module(..., package.seeall)

local path     = require "tundra.path"

DefRule {
  Name = "Lemon",
  Command = "lemon -d$(OUTDIR) $(<)",
  ConfigInvariant = true,

  Blueprint = {
    Source = { Required = true, Type = "string" },
    OutputDir = { Required = false, Type = "string" },
  },

  Setup = function (env, data)
    local src = data.Source
    local base_name = path.drop_suffix(src)
    env:set('OUTDIR', data.OutputDir or '.')
    local gen_c = '$(OUTDIR)$(SEP)' .. base_name .. '.c'
    local gen_h = '$(OUTDIR)$(SEP)' .. base_name .. '.h'
    local gen_out = '$(OUTDIR)$(SEP)' .. base_name .. '.out'
    return {
      InputFiles = { src },
      OutputFiles = { gen_c, gen_h, gen_out },
    }
  end,
}
