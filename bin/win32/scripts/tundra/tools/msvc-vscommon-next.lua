-- for usage with Visual Studio 2017 and later, 
-- does not work with earlier versions of Visual Studio

-- see https://blogs.msdn.microsoft.com/vcblog/2016/10/07/compiler-tools-layout-in-visual-studio-15/

module(..., package.seeall)

local native = require "tundra.native"
local native_path = require "tundra.native.path"

local vs_products = {
  "BuildTools", --default
  "Community",
  "Professional",
  "Enterprise"
}

local function find_vc_tools(installation_path, version, vs_product, search_set)
  local path, stat

  path = native_path.join(installation_path, version)
  path = native_path.join(path, vs_product)
  path = native_path.join(path, "VC\\Auxiliary\\Build\\vcvarsall.bat")

  stat = native.stat_file(path)
  if stat.exists then
    return path
  end
  search_set[#search_set+1] = path
  return nil
end

local supported_arch_mappings = {
  ["x86"] = "x86",
  ["amd64"] = "amd64",
  ["arm"] = "arm",
  ["arm64"] = "arm64",
  
  --alias
  ["x64"] = "amd64", 
}

local supported_arch_tuples = {
  ["x86"] = true,
  ["amd64"] = true,
  ["x86_amd64"] = true,
  ["x86_arm"] = true,
  ["x86_arm64"] = true,
  ["amd64_x86"] = true,
  ["amd64_arm"] = true,
  ["amd64_arm64"] = true,
}

local function get_arch(arch)
  local arch2 = supported_arch_mappings[arch:lower()] 
  return arch2
end

-- dump keys of table into sorted array
local function keys(tbl)
  local ks, i = {}, 1
  for k, _ in pairs(tbl) do
    ks[i] = k
    i = i + 1
  end
  table.sort(ks)
  return ks
end

local function get_arch_tuple(host_arch, target_arch)
  local host_arch2 = get_arch(host_arch)
  local target_arch2 = get_arch(target_arch)

  if host_arch2 == nil then
    error("unknown host architecture '" .. host_arch .. "' expected one of " .. table.concat(keys(supported_arch_mappings), ", "))
  end
  if target_arch2 == nil then
    error("unknown target architecture '" .. target_arch .. "' expected one of " .. table.concat(keys(supported_arch_mappings), ", "))
  end

  if host_arch2 == target_arch2 then
    return host_arch2
  end
  return host_arch2 .. "_" .. target_arch2
end

local vcvars_cache = {}

function apply_msvc_visual_studio(version, env, options)
  if native.host_platform ~= "windows" then
    error("the msvc toolset only works on windows hosts")
  end
  
  tundra.unitgen.load_toolset('msvc', env)

  options = options or {}

  -- these control how vcvarsall.bat is invoked
  local target_arch = options.TargetArch or "x64"
  local host_arch = options.HostArch or "x64"
  local arch_tuple = get_arch_tuple(host_arch, target_arch)
  local platform_type = options.PlatformType --default empty ({empty} | store | uwp)
  local sdk_version = options.SdkVersion --default from vcvarsall.bat (otherwise request a specific version through this option)
  local installation_path = options.InstallationPath or "C:\\Program Files (x86)\\Microsoft Visual Studio"
  local vs_product = options.Product
  local vcvarsall_bat = options.VCVarsPath --override the search strategy and use a specific 'vcvars' bat file
  local search_set = {}
  
  if arch_tuple == nil then
    error("unsupported host/target architecture " .. arch_tuple)
  end

  if vcvarsall_bat == nil then
    -- don't assume that everyone is using the same edition of Visual Studio
    -- unless a specific edition (i.e. product) has been requested, look for
    -- the first setup that has Visual C++ compiler toolset installed
    if vs_product == nil then
      for _, vs_product in pairs(vs_products) do
        vcvarsall_bat = find_vc_tools(installation_path, version, vs_product, search_set)
        if vcvarsall_bat ~= nil then
          break
        end
      end
    else
      vcvarsall_bat = find_vc_tools(installation_path, version, vs_product, search_set)
    end
  else
    local stat = native.stat_file(vcvarsall_bat)
    if not stat.exists then
      error("cannot find the vcvars batch file: " .. vcvarsall_bat)
    end
  end

  if vcvarsall_bat == nil then
    error("cannot find the Visual C++ compiler toolset in any of the following locations: \n  " .. table.concat(search_set, "\n  ") .. "\nCheck that either Desktop development with C++ or Visual C++ build tools is installed. You can customize the InstallationPath or Product options to load a specific instance of Visual Studio " .. version .. " from a specific location. If Product is not specified it will look for the first instance of the Visual C++ compiler toolset in the default installation path 'C:\\Program Files (x86)\\Microsoft Visual Studio' in the following order " .. table.concat(vs_products, ", ") .. " using the tripplet (InstallationPath, '" .. version .. "', Product)")
  end

  -- now to the fun part, the vcvarsall.bat script is quite customizable
  -- we can request the environment from it, let it do the hard part and
  -- then diff and merge things back into our environment

  -- Syntax:
  --     vcvarsall.bat [arch] [platform_type] [winsdk_version] [-vcvars_ver=vc_version]
  -- where :
  --     [arch]: x86 | amd64 | x86_amd64 | x86_arm | x86_arm64 | amd64_x86 | amd64_arm | amd64_arm64
  --     [platform_type]: {empty} | store | uwp
  --     [winsdk_version] : full Windows 10 SDK number (e.g. 10.0.10240.0) or "8.1" to use the Windows 8.1 SDK.
  --     [vc_version] : "14.0" for VC++ 2015 Compiler Toolset | {empty} for default VS 2017 VC++ compiler toolset
  -- 
  -- The store parameter sets environment variables to support Universal Windows Platform application
  -- development and is an alias for 'uwp'.
  -- 
  -- For example:
  --     vcvarsall.bat x86_amd64
  --     vcvarsall.bat x86_amd64 10.0.10240.0
  --     vcvarsall.bat x86_arm uwp 10.0.10240.0
  --     vcvarsall.bat x86_arm onecore 10.0.10240.0 -vcvars_ver=14.0
  --     vcvarsall.bat x64 8.1
  --     vcvarsall.bat x64 store 8.1

  local vcvars_args = { arch_tuple }

  if platform_type ~= nil then
    vcvars_args[#vcvars_args+1] = platform_type
  end

  if sdk_version ~= nil then
    vcvars_args[#vcvars_args+1] = sdk_version
  end

  local vcvars = {}

  local vcvars_a = table.concat(vcvars_args, " ")
  local vcvars_key = vcvarsall_bat .. vcvars_a
  if not vcvars_cache[vcvars_key] then
    local command = "\"" .. vcvarsall_bat .. "\" " .. vcvars_a .. " 1>NUL && set"
    local vcvars_p = io.popen(command)
    for ln in vcvars_p:lines() do
      local split_at = ln:find("=")
      local k = ln:sub(1, split_at-1):upper()
      local v = ln:sub(split_at+1)
      vcvars[k] = v
    end
    vcvars_p:close()
    vcvars_cache[vcvars_key] = vcvars
  else
    vcvars = vcvars_cache[vcvars_key]
  end
 
  --print(table.concat(keys(vcvars), " "))

  -- Force MSPDBSRV.EXE (fixes issues with cl.exe running in parallel and corrupting PDB files)
  env:set("CCOPTS", "/FS")
  env:set("CXXOPTS", "/FS")

  local copy_env_vars = {
    "VSINSTALLDIR",
    "VCINSTALLDIR",
    "INCLUDE",
    "LIB",
    "LIBPATH",
    "PATH",
  }

  for _, k in pairs(copy_env_vars) do
    local v = vcvars[k]
    if v == nil then
      print("Warning: expected environment variable '" .. k .. "' but got nil")
    else
      env:set_external_env_var(k, v)
    end
  end

  -- OK done
end
