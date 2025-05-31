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
    # Путь к установленной OpenCV через vcpkg
    OPENCV_DIR = C:\msys64\mingw64

    INCLUDEPATH += $$OPENCV_DIR/include
    INCLUDEPATH += $$OPENCV_DIR/include/opencv4

    LIBS += -L$$OPENCV_DIR/lib \
            -lopencv_core \
            -lopencv_highgui \
            -lopencv_imgproc \
            -lopencv_imgcodecs \
            -lopencv_videoio

    # Копирование DLL в папку с исполняемым файлом
    QMAKE_POST_LINK += xcopy /Y /D /Q "$$OPENCV_DIR\\bin\\*.dll" "$$OUT_PWD\\"

}
