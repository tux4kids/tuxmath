## Makefile.am for tuxmath/linebreak:
## Process with Automake to create Makefile.in

noinst_LIBRARIES = liblinebreak.a
liblinebreak_a_SOURCES = linebreak.c \
  unistr/u16-mbtouc-aux.c \
  unistr/u16-mbtouc.c \
  unistr/u16-mbtouc-unsafe-aux.c \
  unistr/u16-mbtouc-unsafe.c \
  unistr/u8-mbtouc-aux.c \
  unistr/u8-mbtouc.c \
  unistr/u8-mbtouc-unsafe-aux.c \
  unistr/u8-mbtouc-unsafe.c \
  unistr/u8-uctomb-aux.c \
  unistr/u8-uctomb.c \
  uniwidth/width.c

EXTRA_DIST = lbrkprop.h \
  linebreak.h \
  localcharset.h \
  streq.h \
  unistr.h \
  unitypes.h \
  uniwidth.h \
  xsize.h \
  uniwidth/cjk.h \
  CMakeLists.txt \
  linebreak_config.h.cmake

