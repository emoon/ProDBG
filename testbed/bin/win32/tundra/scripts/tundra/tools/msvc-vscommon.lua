-- msvc-vscommon.lua - utility code for all versions of Visual Studio

module(..., package.seeall)

local native = require "tundra.native"
local os = require "os"

local arch_dirs = {
  ["x86"] = {
    ["x86"] = "\\vc\\bin\\",
    ["x64"] = "\\vc\\bin\\x86_amd64\\",
    ["itanium"] = "\\vc\\bin\\x86_ia64\\",
    ["arm"] = "\\vc\\bin\\x86_arm\\",
  },
  ["x64"] = {
    ["x86"] = "\\vc\\bin\\",
    ["x64"] = "\\vc\\bin\\amd64\\",
    ["itanium"] = "\\vc\\bin\\x86_ia64\\",
    ["arm"] = "\\vc\\bin\\x86_arm\\",
  },
  ["itanium"] = {
    ["x86"] = "\\vc\\bin\\x86_ia64\\",
    ["itanium"] = "\\vc\\bin\\ia64\\",
  },
}

local function get_host_arch()
  local snative = native.getenv("PROCESSOR_ARCHITECTURE")
  local swow = native.getenv("PROCESSOR_ARCHITEW6432", "")
  if snative == "AMD64" or swow == "AMD64" then
    return "x64"
  elseif snative == "IA64" or swow == "IA64" then
    return "itanium";
  else
    return "x86"
  end
end

function apply_msvc_visual_studio(version, env, options)

  if native.host_platform ~= "windows" then
    error("the msvc toolset only works on windows hosts")
  end
  -- Load basic MSVC environment setup first. We're going to replace the paths to
  -- some tools.
  tundra.unitgen.load_toolset('msvc', env)

  options = options or {}

  local target_arch = options.TargetArch or "x86"
  local host_arch = options.HostArch or get_host_arch()

  local vs_key = "SOFTWARE\\Microsoft\\VisualStudio\\" .. version
  local idePath = assert(native.reg_query("HKLM", vs_key, "InstallDir"))
  local rootDir = string.gsub(idePath, "\\Common7\\IDE\\$", "")

  local sdkDir
  local sdkIncludeDir

  local sdkLibDir
  local vcLibDir

  if version ~= "11.0" then
    local sdk_key = "SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows"
    sdkDir = assert(native.reg_query("HKLM", sdk_key, "CurrentInstallFolder"))
    sdkIncludeDir = sdkDir .. "\\INCLUDE"

    if "x86" == target_arch then
      sdkLibDir = "LIB"
      vcLibDir = "LIB"
    elseif "x64" == target_arch then
      sdkLibDir = "LIB\\x64"
      vcLibDir = "LIB\\amd64"
    elseif "itanium" == target_arch then
      sdkLibDir = "LIB\\IA64"
      vcLibDir = "LIB\\IA64"
    end
  else
    -- Hardcode VS2012 to use Windows SDK 8.0
    local sdk_key = "SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v8.0"
    sdkDir = assert(native.reg_query("HKLM", sdk_key, "InstallationFolder"))
    sdkIncludeDir = sdkDir .. "\\include\\um;" .. sdkDir .. "\\include\\shared"

    if "x86" == target_arch then
      sdkLibDir = "lib\\win8\\um\\x86"
      vcLibDir = "lib"
    elseif "x64" == target_arch then
      sdkLibDir = "lib\\win8\\um\\x64"
      vcLibDir = "lib\\amd64"
    elseif "arm" == target_arch then
      sdkLibDir = "lib\\win8\\um\\arm"
      vcLibDir = "lib\\arm"
    end
  end

  local binDir = arch_dirs[host_arch][target_arch]

  if not binDir then
    errorf("can't build target arch %s on host arch %s", target_arch, host_arch)
  end

  local cl_exe = '"' .. rootDir .. binDir .. "cl.exe" ..'"'
  local lib_exe = '"' .. rootDir .. binDir .. "lib.exe" ..'"'
  local link_exe = '"' .. rootDir .. binDir .. "link.exe" ..'"'

  env:set('CC', cl_exe)
  env:set('CXX', cl_exe)
  env:set('LIB', lib_exe)
  env:set('LD', link_exe)

  -- Pickup the Resource Compiler from the current SDK path
  -- (otherwise tundra has to be run from within an environment where 'RC' already is in the path)
  local rc_exe
  if version == "11.0" then
    rc_exe = '"' .. sdkDir .. "bin\\x86\\rc.exe" .. '"'
  else
    rc_exe = '"' .. sdkDir .. "bin\\rc.exe" .. '"'
  end
  env:set('RC', rc_exe)

  -- Expose the required variables to the external environment
  env:set_external_env_var('VSINSTALLDIR', rootDir)
  env:set_external_env_var('VCINSTALLDIR', rootDir .. '\\vc')
  env:set_external_env_var('DevEnvDir', idePath)

  -- Now look for MS SDK associated with visual studio

  env:set_external_env_var("WindowsSdkDir", sdkDir)
  env:set_external_env_var("INCLUDE", sdkIncludeDir .. ";" .. rootDir .. "\\VC\\ATLMFC\\INCLUDE;" .. rootDir .. "\\VC\\INCLUDE")

  local libString = sdkDir .. "\\" .. sdkLibDir .. ";" .. rootDir .. "\\VC\\ATLMFC\\" .. vcLibDir .. ";" .. rootDir .. "\\VC\\" .. vcLibDir
  env:set_external_env_var("LIB", libString)
  env:set_external_env_var("LIBPATH", libString)

  local path = { }
  path[#path + 1] = sdkDir
  path[#path + 1] = idePath

  if "x86" == host_arch then
    path[#path + 1] = rootDir .. "\\VC\\Bin"
  elseif "x64" == host_arch then
    path[#path + 1] = rootDir .. "\\VC\\Bin\\amd64"
  elseif "itanium" == host_arch then
    path[#path + 1] = rootDir .. "\\VC\\Bin\\ia64"
  elseif "arm" == host_arch then
    path[#path + 1] = rootDir .. "\\VC\\Bin\\arm"
  end
  path[#path + 1] = rootDir .. "\\Common7\\Tools"

  path[#path + 1] = env:get_external_env_var('PATH') 

  env:set_external_env_var("PATH", table.concat(path, ';'))
end
