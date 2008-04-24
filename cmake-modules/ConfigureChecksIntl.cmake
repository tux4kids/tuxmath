if (NOT INTL_CHECKS_DONE)
  include(CheckIncludeFiles)
  include(CheckSymbolExists)
  include(CheckCSourceCompiles)
  include(CheckTypeSize)
  include(MacroBoolTo01)

  #message(STATUS "CMAKE_REQUIRED_INCLUDES: ${CMAKE_REQUIRED_INCLUDES}")
  #set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} /usr/include)
  #message(STATUS "CMAKE_REQUIRED_INCLUDES: ${CMAKE_REQUIRED_INCLUDES}")
  
  #check_include_files(sys/types.h HAVE_SYS_TYPES_H)
  #check_include_files(inttypes.h HAVE_INTTYPES_H)
  #check_include_files(stdint.h HAVE_STDINT_H)
  #check_include_files(stddef.h HAVE_STDDEF_H)
  
  check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)
  check_symbol_exists(asprintf "stdio.h" HAVE_ASPRINTF)
  check_symbol_exists(wprintf "stdio.h" HAVE_WPRINTF)
  check_symbol_exists(printf "stdio.h" HAVE_POSIX_PRINTF)

  # Alternate for HAVE_POSIX_PRINTF: which is right?
  check_c_source_compiles("
  #include <stdio.h>
  #include <string.h>
  static char format[] = { '%', '2', '$', 'd', ' ', '%', '1', '$', 'd', '\0' };
  static char buf[100];
  int main ()
  {
    sprintf (buf, format, 33, 55);
  }" HAVE_POSIX_PRINTF)


  check_symbol_exists(intmax_t "inttypes.h" HAVE_INTTYPES_H_WITH_UINTMAX)
  check_symbol_exists(intmax_t "stdint.h" HAVE_INTTYPES_H_WITH_UINTMAX)
  check_symbol_exists(uintmax_t "stdint.h" HAVE_STDINT_H_WITH_UINTMAX)
  check_symbol_exists(LC_MESSAGES "locale.h" HAVE_LC_MESSAGES)
  check_symbol_exists(fcntl "stdio.h" HAVE_FCNTL)

  check_type_size(intmax_t INTMAX_T)
  message("HIT ${HAVE_INTMAX_T}, IT ${INTMAX_T}")
  macro_bool_to_01(HAVE_STDINT_H_WITH_UINTMAX HAVE_UINTMAX_T)

  set(INTL_CHECKS_DONE TRUE)
endif (NOT INTL_CHECKS_DONE)
