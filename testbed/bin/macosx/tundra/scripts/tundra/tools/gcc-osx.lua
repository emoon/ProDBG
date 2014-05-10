module(..., package.seeall)

function apply(env, options)
  -- load the generic GCC toolset first
  tundra.unitgen.load_toolset("gcc", env)

  env:set_many {
    ["NATIVE_SUFFIXES"] = { ".c", ".cpp", ".cc", ".cxx", ".m", ".mm", ".a", ".o" },
    ["CXXEXTS"] = { "cpp", "cxx", "cc", "mm" },
    ["FRAMEWORKS"] = "",
    ["SHLIBPREFIX"] = "lib",
    ["SHLIBOPTS"] = "-shared",
    ["_OS_CCOPTS"] = "$(FRAMEWORKS:p-F)",
    ["SHLIBCOM"] = "$(LD) $(SHLIBOPTS) $(LIBPATH:p-L) $(LIBS:p-l) $(FRAMEWORKS:p-framework ) -o $(@) $(<)",
    ["PROGCOM"] = "$(LD) $(PROGOPTS) $(LIBPATH:p-L) $(LIBS:p-l)  $(FRAMEWORKS:p-framework ) -o $(@) $(<)",
    ["OBJCCOM"] = "$(CCCOM)", -- objc uses same commandline
    ["NIBCC"] = "ibtool --output-format binary1 --compile $(@) $(<)",
  }
end
