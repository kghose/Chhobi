#-------------------------------------------------
#
# Project created by QtCreator 2011-11-04T16:41:09
#
#-------------------------------------------------

ICON = Icons/chhobi.icns

QT += core gui sql
LIBS += -L/usr/local/lib/ -lexiv2


TARGET = Chhobi
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    photoribbon.cpp \
    photo.cpp \
    exivmanager.cpp \
    database.cpp

HEADERS  += mainwindow.h \
    photoribbon.h \
    photo.h \
    exivmanager.h \
    imagerotate.h \
    database.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    Readme.md

RESOURCES += \
    resources.qrc

















































