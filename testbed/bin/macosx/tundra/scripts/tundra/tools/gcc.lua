module(..., package.seeall)

function apply(env, options)
  -- load the generic C toolset first
  tundra.unitgen.load_toolset("generic-cpp", env)

  env:set_many {
    ["NATIVE_SUFFIXES"] = { ".c", ".cpp", ".cc", ".cxx", ".a", ".o" },
    ["OBJECTSUFFIX"] = ".o",
    ["LIBPREFIX"] = "lib",
    ["LIBSUFFIX"] = ".a",
    ["_GCC_BINPREFIX"] = "",
    ["CC"] = "$(_GCC_BINPREFIX)gcc",
    ["CXX"] = "$(_GCC_BINPREFIX)g++",
    ["LIB"] = "$(_GCC_BINPREFIX)ar",
    ["LD"] = "$(_GCC_BINPREFIX)gcc",
    ["_OS_CCOPTS"] = "",
    ["_OS_CXXOPTS"] = "",
    ["CCCOM"] = "$(CC) $(_OS_CCOPTS) -c $(CPPDEFS:p-D) $(CPPPATH:f:p-I) $(CCOPTS) $(CCOPTS_$(CURRENT_VARIANT:u)) -o $(@) $(<)",
    ["CXXCOM"] = "$(CXX) $(_OS_CXXOPTS) -c $(CPPDEFS:p-D) $(CPPPATH:f:p-I) $(CXXOPTS) $(CXXOPTS_$(CURRENT_VARIANT:u)) -o $(@) $(<)",
    ["PROGOPTS"] = "",
    ["PROGCOM"] = "$(LD) $(PROGOPTS) $(LIBPATH:p-L) -o $(@) $(<) $(LIBS:p-l)",
    ["PROGPREFIX"] = "",
    ["LIBOPTS"] = "",
    ["LIBCOM"] = "$(LIB) -rs $(LIBOPTS) $(@) $(<)",
    ["SHLIBPREFIX"] = "lib",
    ["SHLIBOPTS"] = "-shared",
    ["SHLIBCOM"] = "$(LD) $(SHLIBOPTS) $(LIBPATH:p-L) -o $(@) $(<) $(LIBS:p-l)",
  }
end
