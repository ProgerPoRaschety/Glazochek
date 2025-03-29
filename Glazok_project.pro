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
linux {
    INCLUDEPATH += /usr/include/opencv4
    LIBS += -lopencv_core \
            -lopencv_highgui \
            -lopencv_videoio \
            -lopencv_imgproc

    # Если нужно VTK
    LIBS += -lvtkCommonCore \
            -lvtkInteractionStyle \
            -lvtkFiltersCore
}

win32 {
    OPENCV_DIR = C:/opencv/build
    INCLUDEPATH += $$OPENCV_DIR/include
    LIBS += -L$$OPENCV_DIR/x64/vc16/lib \
            -lopencv_world451
}

FORMS += \
    mainwindow.ui
