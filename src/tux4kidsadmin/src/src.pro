TARGET = ../tux4kidsadmin
TEMPLATE = app
INCLUDEPATH	+= ../plugins/

SOURCES += main.cpp \
    mainWindow.cpp \
    mainController.cpp

HEADERS += mainWindow.h \
    mainController.h

FORMS += mainWindow.ui

# Uncomment to use GCOV
# QMAKE_CXXFLAGS+=-fprofile-arcs -ftest-coverage

