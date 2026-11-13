QT += core gui qml quick quickcontrols2 dbus

CONFIG += c++17 release link_pkgconfig
PKGCONFIG += poppler-qt6

TARGET = splinknote
TEMPLATE = app

HEADERS += \
    src/backend.h \
    src/canvasitem.h \
    src/filepicker.h \
    src/portalfilepicker.h \
    src/theme.h \
    src/translator.h

SOURCES += \
    src/main.cpp \
    src/backend.cpp \
    src/canvasitem.cpp \
    src/portalfilepicker.cpp \
    src/theme.cpp \
    src/translator.cpp

RESOURCES += src/resources.qrc
