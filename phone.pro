#-------------------------------------------------
#
# Project created by QtCreator 2016-08-12T19:24:22
#
#-------------------------------------------------

QT += core gui sql

QMAKE_CXX += -std=gnu++11 -O2

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = phonebook

TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
