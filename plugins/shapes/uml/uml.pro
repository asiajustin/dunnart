
QT           += xml svg
TEMPLATE      = lib
CONFIG       += qt plugin
TARGET        = $$qtLibraryTarget(org.dunnart.UMLShapesPlugin)

include(../../../common_options.qmake)
include(../shape_plugin_options.pri)

HEADERS       = umlclass.h \
    editumlclassinfodialog.h \
    umlnote.h \
    umlpackage.h
SOURCES       = plugin.cpp umlclass.cpp \
    editumlclassinfodialog.cpp \
    umlnote.cpp \
    umlpackage.cpp

FORMS += \
    editumlclassinfodialog.ui
