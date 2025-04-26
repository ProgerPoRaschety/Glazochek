QT += core gui widgets
CONFIG += c++11

TARGET = Glazok_project
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    cv_webcam_capture.cpp \
    motion_detector.cpp

HEADERS += \
    mainwindow.h \
    cv_webcam_capture.h \
    motion_detector.h

FORMS += \
    mainwindow.ui

linux {
    INCLUDEPATH += /usr/include/opencv4
    LIBS += -lopencv_core \
            -lopencv_highgui \
            -lopencv_videoio \
            -lopencv_imgproc \
            -lopencv_imgcodecs
}

win32 {
    OPENCV_DIR = C:/Users/Antonio/vcpkg/installed/x64-windows

    INCLUDEPATH += $$OPENCV_DIR/include
    INCLUDEPATH += $$OPENCV_DIR/include/opencv4

    LIBS += -L$$OPENCV_DIR/lib \
            -lopencv_core \
            -lopencv_highgui \
            -lopencv_imgproc \
            -lopencv_imgcodecs \
            -lopencv_videoio

    QMAKE_POST_LINK += copy /Y $$OPENCV_DIR\\bin\\*.dll $$OUT_PWD\\
}
