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
FORMS += \
    mainwindow.ui
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
    # Путь к установленной OpenCV через vcpkg
    OPENCV_DIR = C:/Users/Antonio/vcpkg/installed/x64-windows

    INCLUDEPATH += $$OPENCV_DIR/include
    INCLUDEPATH += $$OPENCV_DIR/include/opencv4

    LIBS += -L$$OPENCV_DIR/lib \
            -lopencv_core \
            -lopencv_highgui \
            -lopencv_imgproc \
            -lopencv_imgcodecs \
            -lopencv_videoio

    # Копирование DLL (исправленный путь)
    QMAKE_POST_LINK += copy /Y $$OPENCV_DIR\\bin\\*.dll $$DESTDIR\\
}

