#-------------------------------------------------
#
# Project created by QtCreator 2018-09-04T23:50:48
#
#-------------------------------------------------

QT       += core gui
QT += sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hummingbird
TEMPLATE = app
RC_FILE += \
    app.rc

SOURCES += main.cpp\
        mainwindow.cpp \
    hummingbird.cpp \
    hummingbirddb.cpp \
    hummingbirdp.pb.cc

HEADERS  += mainwindow.h \
    hummingbird.h \
    hummingbirddb.h \
    hummingbirdp.pb.h

FORMS    += mainwindow.ui

win32:INCLUDEPATH += "C:\Program Files (x86)\GnuWin32\include"
win32:INCLUDEPATH += "C:\Program Files (x86)\log4cplus\include"
win32:INCLUDEPATH += "C:\Program Files (x86)\protobuf\include"
win32:INCLUDEPATH += "C:\Program Files (x86)\ZeroMQ 4.0.4\include"
win32:LIBS += "C:\Program Files (x86)\GnuWin32\lib\libz.a"
win32:LIBS += "C:\Program Files (x86)\log4cplus\lib\liblog4cplusU.dll.a"
win32:LIBS += "C:\Program Files (x86)\protobuf\lib\libprotobuf-lite.a"
win32:LIBS += "C:\Program Files (x86)\ZeroMQ 4.0.4\lib\libzmq-v120-mt-4_0_4.lib"

unix:INCLUDEPATH += "/usr/local/include"
unix:LIBS += "/usr/local/lib/libzmq.dylib"
unix:LIBS += "/usr/local/lib/libz.dylib"
unix:LIBS += "/usr/local/lib/libprotobuf-lite.dylib"
unix:LIBS += "/usr/local/lib/liblog4cplus.dylib"

QMAKE_CXXFLAGS += -Wno-deprecated-declarations -Wno-unused-parameter -Wno-cpp
