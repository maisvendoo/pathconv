TEMPLATE = app

TARGET = pathconv

QT += core

DESTDIR = ../bin

INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
