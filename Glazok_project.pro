QT += core gui widgets
CONFIG += c++11

TARGET = Glazok_project
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    cv_webcam_capture.cpp

HEADERS += \
    mainwindow.h \
    cv_webcam_capture.h

# OpenCV
unix:!macx {
    INCLUDEPATH += /usr/include/opencv4
    LIBS += -lopencv_core -lopencv_highgui -lopencv_videoio -lopencv_imgproc
}
