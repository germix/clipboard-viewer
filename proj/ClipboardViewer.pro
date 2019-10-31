#-------------------------------------------------
#
# Project created by QtCreator 2011-07-04T08:13:23
#
#-------------------------------------------------

QT       += core gui widgets winextras

TEMPLATE = app
LIBS = -lole32 -lgdi32

QT += svg
QT += xml

DEFINES += ICON_STATUSBAR

#DEFINES += USE_OEMTEXT
DEFINES += USE_SPLITTER
DEFINES += USE_SPLITTER_CONTAINER

#---------------------------------------------------------------------------------------------------
# Versión
#---------------------------------------------------------------------------------------------------

DEFINES += MAJOR_VERSION=1
DEFINES += MINOR_VERSION=1

#---------------------------------------------------------------------------------------------------
# Target
#---------------------------------------------------------------------------------------------------
CONFIG(debug, debug|release) {
TARGET = ClipboardViewer_d
} else {
TARGET = ClipboardViewer
}

#---------------------------------------------------------------------------------------------------
# Directorio de destino
#---------------------------------------------------------------------------------------------------
DESTDIR = ..\bin

#---------------------------------------------------------------------------------------------------
# Archivos de código
#---------------------------------------------------------------------------------------------------

SOURCES +=		src/main.cpp\
                src/MainWindow.cpp \
				src/HexView.cpp \
				src/ClipboardHandler.cpp \
				src/ClipboardHandlerHtml.cpp \
				src/ClipboardHandlerText.cpp \
				src/ClipboardHandlerImage.cpp \
				src/ClipboardHandlerPath.cpp \
				src/panels/SvgPanel.cpp \
				src/panels/ImagePanel.cpp \
				src/ClipboardHandlerSVG.cpp \
				src/ClipboardHandlerObjectDescriptor.cpp \
				src/ClipboardHandlerAxialisDIB32Data.cpp \
				src/ClipboardHandlerLocale.cpp \
				src/ClipboardHandlerCSV.cpp \
                src/ClipboardHandlerXML.cpp \
				src/ClipboardHandlerUniformResourceLocator.cpp \
				src/ClipboardHandlerPreferredDropEffect.cpp \
                src/AboutDialog.cpp \
    src/Languages.cpp \
    src/dom/domitem.cpp \
    src/dom/dommodel.cpp \
    src/ClipboardHandlerFileGroupDescriptor.cpp \
    src/utils.cpp \
    src/ClipboardHandlerShellObjectOffsets.cpp \
    src/ClipboardHandlerAsyncFlag.cpp \
    src/ClipboardHandlerDIF.cpp \
    src/WinClipboard.cpp \
    src/ClipboardFormat.cpp

HEADERS  +=		src/MainWindow.h \
				src/HexView.h \
				src/ClipboardHandler.h \
				src/panels/SvgPanel.h \
				src/panels/ImagePanel.h \
                src/AboutDialog.h \
    src/Languages.h \
    src/dom/domitem.h \
    src/dom/dommodel.h \
    src/WinClipboard.h \
    src/ClipboardFormat.h \
    src/utils.h

FORMS    +=		src/MainWindow.ui \
                src/AboutDialog.ui

#---------------------------------------------------------------------------------------------------
# Archivos de recursos
#---------------------------------------------------------------------------------------------------

RESOURCES += res/resource.qrc

win32:RC_FILE = res/resource_win32.rc

#---------------------------------------------------------------------------------------------------
# Archivos de traducción
#---------------------------------------------------------------------------------------------------
TRANSLATIONS =	translate/cbvw_en.ts \
                translate/cbvw_es.ts

#---------------------------------------------------------------------------------------------------
# Otros archivos
#---------------------------------------------------------------------------------------------------

OTHER_FILES += \
    doc/ClipboardViewer.txt
