VT_VERSION = 0.95
DEFINES += VT_VERSION=$$VT_VERSION

CONFIG += c++11

QT += core
QT += gui
QT += widgets
QT += serialport
QT += network
QT += printsupport
QT += quick

TARGET = application
TEMPLATE = app

# Serial port available
DEFINES += HAS_SERIALPORT

# Bluetooth available
#DEFINES += HAS_BLUETOOTH

contains(DEFINES, HAS_SERIALPORT) {
    QT += serialport
}

contains(DEFINES, HAS_BLUETOOTH) {
    QT += bluetooth
}

SOURCES += main.cpp\
    commands.cpp \
    configparam.cpp \
    configparams.cpp \
    packet.cpp \
    openroadinterface.cpp \
    vbytearray.cpp \
    utility.cpp \
    tcpserversimple.cpp

HEADERS  += commands.h \
    configparam.h \
    configparams.h \
    datatypes.h \
    packet.h \
    openroadinterface.h \
    vbytearray.h \
    utility.h \
    tcpserversimple.h
    
contains(DEFINES, HAS_BLUETOOTH) {
    SOURCES += bleuart.cpp
    HEADERS += bleuart.h
}

include(widgets/widgets.pri)
include(lzokay/lzokay.pri)

RESOURCES += \
    qml.qrc \
    res_config.qrc

