TEMPLATE	= lib
CONFIG		+= plugin
INCLUDEPATH	+= ../

HEADERS		= dummy.h \

SOURCES		= dummy.cpp \

TARGET		= $$qtLibraryTarget(dummyPlugin)
DESTDIR		= ../

# Uncomment to use GCOV
# QMAKE_CXXFLAGS+=-fprofile-arcs -ftest-coverage

