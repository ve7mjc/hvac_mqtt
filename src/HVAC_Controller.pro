QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

include(mqtt_adapter/mqttadapter.pri)

SOURCES += main.cpp \
    controller.cpp

HEADERS += \
    controller.h
