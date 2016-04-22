#-------------------------------------------------
#
# Project created by QtCreator 2015-12-15T14:34:32
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT       += printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TMutomo
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    dialog.cpp \
    viewconstr.cpp \
    settings.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    dialog.h \
    viewconstr.h \
    settings.h

FORMS    += mainwindow.ui \
    dialog.ui \
    settings.ui

CONFIG += mobility c++11
MOBILITY = 

DISTFILES += \
    StyleSheet.qss

RESOURCES += \
    StyleSheet.qrc

