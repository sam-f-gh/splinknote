QT += core gui quick quickcontrols2 testlib dbus
CONFIG += c++17 testcase link_pkgconfig
PKGCONFIG += poppler-qt6
TARGET = splinknote_tests
TEMPLATE = app

INCLUDEPATH += ../src

HEADERS += \
    ../src/backend.h \
    ../src/canvasitem.h \
    ../src/filepicker.h \
    ../src/portalfilepicker.h \
    ../src/theme.h \
    ../src/translator.h

SOURCES += \
    splinknote_tests.cpp \
    ../src/backend.cpp \
    ../src/canvasitem.cpp \
    ../src/portalfilepicker.cpp \
    ../src/theme.cpp \
    ../src/translator.cpp

RESOURCES += ../src/resources.qrc
