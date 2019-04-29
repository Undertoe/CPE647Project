QT -= gui

QT += network mqtt core

CONFIG += c++1z console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += /Users/terryn.fredrickson/Documents/Libraries/CPP_Libs/include/
INCLUDEPATH += NotificationTypes/
INCLUDEPATH += Sensors/

LIBS += /Users/terryn.fredrickson/Documents/Libraries/CPP_Libs/lib/libCoreLib.a

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
    server.cpp \
    ingester.cpp \
    processors.cpp \
    gcinterface.cpp \
    sensorinfo.cpp \
    globalcontroller.cpp \
    datastore.cpp \
    sensorprocessing.cpp \
    Sensors/imusensor.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    server.h \
    ingester.h \
    processors.h \
    gcinterface.h \
    sensorinfo.h \
    globalcontroller.h \
    datastore.h \
    sensorprocessing.h \
    semaphorefix.h \
    circularlockedqueue.h \
    gcnetworkpostbuilder.h \
    gclouddata.h \
    NotificationTypes/landingnotification.h \
    Sensors/imusensor.h \
    NotificationTypes/requestphonekeys.h \
    NotificationTypes/publishdata.h
