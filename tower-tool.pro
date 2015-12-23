QT += core network
QT -= gui

TARGET = tower-tool
CONFIG += console c++11
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    tower.cpp

HEADERS += \
    tower.h

TRANSLATIONS += i18n/tower-tool_zh_CN.ts

RESOURCES += \
    resource.qrc
