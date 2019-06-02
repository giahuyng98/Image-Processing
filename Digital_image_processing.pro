#-------------------------------------------------
#
# Project created by QtCreator 2019-04-17T23:41:22
#
#-------------------------------------------------

QT       += core gui
QT       += charts
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Digital_image_processing
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    imagelabel.cpp \
        main.cpp \
        app.cpp \
    imageprocessing.cpp \
    mat3x3.cpp

HEADERS += \
        app.h \
    imagelabel.h \
    imageprocessing.h \
    mat3x3.h

FORMS += \
        app.ui \
    mat3x3.ui

INCLUDEPATH += \
    C:/OpenCV/include \

LIBS += \
    -LC:/OpenCV/x64/mingw/lib/

LIBS += \
    -llibopencv_core400 -llibopencv_imgcodecs400 -llibopencv_imgproc400 -llibopencv_ml400


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
