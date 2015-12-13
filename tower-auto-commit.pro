QT += core network
QT -= gui

TARGET = tower-auto-commit
CONFIG += console c++11
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    weekly.cpp

HEADERS += \
    weekly.h

