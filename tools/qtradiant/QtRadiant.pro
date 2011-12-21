#-------------------------------------------------
#
# Project created by QtCreator 2011-12-18T13:08:48
#
#-------------------------------------------------

CODECFORTR = UTF-8

QT += core gui opengl

TARGET = QtRadiant
TEMPLATE = app

MOC_DIR = src/moc/
UI_DIR = src/

RESOURCES += QtRadiant.qrc

SOURCES += \
    src/main.cpp \
    src/Radiant3DWidget.cpp \
    src/MainWindow.cpp \
	src/CameraWindow.cpp \
    src/InspectorConsoleDock.cpp \
    src/InspectorEntityDock.cpp \
    src/InspectorMediaDock.cpp \
    src/InspectorTexturesDock.cpp \
    src/EntitiesDialog.cpp \
    src/MapInfoDialog.cpp \
    src/InspectorSurfaceDialog.cpp \
    src/FindReplaceDialog.cpp

HEADERS += \
    src/Radiant3DWidget.h \
    src/MainWindow.h \
	src/CameraWindow.h \
    src/InspectorConsoleDock.h \
    src/InspectorEntityDock.h \
    src/InspectorMediaDock.h \
    src/InspectorTexturesDock.h \
    src/EntitiesDialog.h \
    src/MapInfoDialog.h \
    src/InspectorSurfaceDialog.h \
    src/FindReplaceDialog.h

FORMS   += \
    src/MainWindow.ui \
	src/CameraWindow.ui \
    src/InspectorConsoleDock.ui \
    src/InspectorEntityDock.ui \
    src/InspectorMediaDock.ui \
    src/InspectorTexturesDock.ui \
    src/EntitiesDialog.ui \
    src/MapInfoDialog.ui \
    src/InspectorSurfaceDialog.ui \
    src/FindReplaceDialog.ui

TRANSLATIONS = \
    trans/QtRadiant.pl_PL.ts

win32  {
	INCLUDEPATH += "../qtradiant/"
}

