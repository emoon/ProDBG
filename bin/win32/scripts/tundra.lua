require "strict"

local boot = require "tundra.boot"
local gent = require "tundra.gen_template"

local actions = {
  ['generate-dag'] = function(build_script, json_file)
    assert(build_script, "need a build script name")
    boot.generate_dag_data(build_script, json_file)
  end,

  ['generate-ide-files'] = function(build_script, ide_script)
    assert(build_script, "need a build script name")
    assert(ide_script, "need a generator name")
    boot.generate_ide_files(build_script, ide_script)
  end,

  ['create-template-file'] = gent.generate_template_file,

  ['selftest'] = function()
    require "tundra.selftest"
  end
}

local function main(action_name, ...)
  assert(action_name, "need an action")

  local action = actions[action_name]
    assert(action, "unknown action '" .. action_name .. "'")

  -- check if debugger was requested
  for i, v in ipairs(arg) do
    if v == "--lua-debugger" then
      table.remove(arg, i)
      require "tundra.debugger"
      pause()
      break
    end
  end

  action(unpack(arg))
end

return {
  main = main
}
