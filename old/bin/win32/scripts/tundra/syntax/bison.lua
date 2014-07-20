module(..., package.seeall)

local nodegen = require "tundra.nodegen"
local path = require "tundra.path"
local depgraph = require "tundra.depgraph"

local _bison_mt = nodegen.create_eval_subclass {}

local bison_blueprint = {
  Source = { Required = true, Type = "string" },
  OutputFile = { Required = false, Type = "string" },
  TokenDefines = { Required = false, Type = "boolean" },
}

function _bison_mt:create_dag(env, data, deps)
  local src = data.Source
  local out_src
  if data.OutputFile then
    out_src = "$(OBJECTDIR)$(SEP)" .. data.OutputFile
  else
    local targetbase = "$(OBJECTDIR)$(SEP)bisongen_" .. path.get_filename_base(src)
    out_src = targetbase .. ".c"
  end
  local defopt = ""
  local outputs = { out_src }
  if data.TokenDefines then
    local out_hdr = path.drop_suffix(out_src) .. ".h"
    defopt = "--defines=" .. out_hdr
    outputs[#outputs + 1] = out_hdr
  end
  return depgraph.make_node {
    Env = env,
    Pass = data.Pass,
    Label = "Bison $(@)",
    Action = "$(BISON) $(BISONOPT) " .. defopt .. " --output-file=$(@:[1]) $(<)",
    InputFiles = { src },
    OutputFiles = outputs,
    Dependencies = deps,
  }
end

nodegen.add_evaluator("Bison", _bison_mt, bison_blueprint)
