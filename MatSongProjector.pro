CONFIG += qt \
    release \
    warn_on
QT += core \
    gui \
    widgets
SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/extextwindow.cpp \
    src/circlewidget.cpp \
    src/projectorwindow.cpp
HEADERS += src/mainwindow.h \
    src/extextwindow.h \
    src/circlewidget.h \
    src/projectorwindow.h
FORMS += src/mainwindow.ui
TRANSLATIONS += resources/tr/MatSongProjector.bg.ts
RESOURCES += resources/resources.qrc
RC_FILE = resources/resources.rc
DESTDIR = bin
MOC_DIR = build
OBJECTS_DIR = build
UI_DIR = build
TEMPLATE = app
TARGET = MatSongProjector
