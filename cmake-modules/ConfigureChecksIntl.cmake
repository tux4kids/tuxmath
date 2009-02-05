# This is a relatively minimal set of checks needed to get the
# gettext-runtime library to compile.  It's possible that more extensive
# checks would optimize it.

if (NOT INTL_CHECKS_DONE)
  include(CheckIncludeFiles)
  include(CheckSymbolExists)
  include(CheckCSourceCompiles)
  include(CheckCSourceRuns)
  include(CheckTypeSize)

  # Set _GNU_SOURCE appropriately; this is required for certain checks
  # below (e.g., asprintf, stpcpy, mempcpy).
  check_c_source_compiles("
  #include <features.h>
  #ifdef __GNU_LIBRARY__
  int main() {return 0;} 
  #endif
  " HAVE_GNU_SOURCE)
  if (HAVE_GNU_SOURCE)
    set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
  endif()

  # These first three are necessary if the check_type_size calls are to work
  check_include_files(sys/types.h HAVE_SYS_TYPES_H)
  check_include_files(stdint.h HAVE_STDINT_H)
  check_include_files(stddef.h HAVE_STDDEF_H)
  # Other checks (in some cases only for check_symbol_exists commands)
  check_include_files(inttypes.h HAVE_INTTYPES_H)
  check_include_files(unistd.h HAVE_UNISTD_H)

  # The next 4 checks are required for the conversion of libgnuintl.h.in
  # to libintl.h and libgnuintl.h
  check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)
  check_symbol_exists(asprintf "stdio.h" HAVE_ASPRINTF)
  check_symbol_exists(wprintf "stdio.h" HAVE_WPRINTF)
  check_c_source_runs("
  #include <stdio.h>
  #include <string.h>
  static char *format = \"%2$d %1$d\";
  static char buf[100];
  int main ()
  {
    sprintf (buf, format, 33, 55);
    return (strcmp (buf, \"55 33\") != 0);
  }" HAVE_POSIX_PRINTF)


  check_symbol_exists(LC_MESSAGES "locale.h" HAVE_LC_MESSAGES)
  check_symbol_exists(fcntl "fcntl.h" HAVE_FCNTL)
  check_symbol_exists(stpcpy "string.h" HAVE_STPCPY)
  check_symbol_exists(mempcpy "string.h" HAVE_MEMPCPY)
  check_symbol_exists(getcwd "unistd.h" HAVE_GETCWD)
  check_symbol_exists(alloca "alloca.h" HAVE_ALLOCA)
  check_c_source_compiles("
  #include <inttypes.h>
  #include <sys/types.h>
  int main() {uintmax_t tmp; return 0;}
  " HAVE_INTTYPES_H_WITH_UINTMAX)
  check_c_source_compiles("
  #include <stdint.h>
  #include <sys/types.h>
  int main() {uintmax_t tmp; return 0;}
  " HAVE_STDINT_H_WITH_UINTMAX)
  

  check_type_size(intmax_t INTMAX_T)
  check_type_size(uintmax_t UINTMAX_T)

  set(INTL_CHECKS_DONE TRUE)
endif (NOT INTL_CHECKS_DONE)
