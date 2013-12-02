# Some syntactic sugar to make scripts more readable
#
# Brendan Luchen, 2010

# Set a variable only if it has not already been set
# Example: gentle_set(CMAKE_BUILD_TYPE DEBUG)
macro(t4k_gentle_set var val)
  if (NOT DEFINED var)
    set(var val)
  endif (NOT DEFINED var)
endmacro(t4k_gentle_set)

# Propagate a CMake variable to the C preprocessor
# Example: include_definition(HAVE_ICONV)
function(t4k_include_definition name)
  if (${name})
    add_definitions(-D${name}=1)
  else (${name})
    add_definitions(-D${name}=0)
  endif(${name})
endfunction(t4k_include_definition)
