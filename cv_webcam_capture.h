#ifndef CV_WEBCAM_CAPTURE_H
#define CV_WEBCAM_CAPTURE_H

#include <QObject>
#include <QTimer>
#include <QImage>
#include <QElapsedTimer>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include "motion_detector.h"

class CVWebcamCapture : public QObject
{
    Q_OBJECT
public:
    explicit CVWebcamCapture(QObject *parent = nullptr);
    ~CVWebcamCapture();

    bool start_camera(int camera_index = 0);
    void stop_camera();
    void setSensitivity(int level);
    bool isCameraOpened() const { return m_cameraOpened; }

signals:
    void new_frame(QImage frame, double fps, bool motionDetected);
    void camera_error(QString message);

private slots:
    void process_frame();

private:
    QElapsedTimer m_motionCaptureTimer;
    QElapsedTimer m_noMotionCaptureTimer;
    const int MOTION_CAPTURE_INTERVAL = 3000; // 3 seconds in milliseconds
    const int NO_MOTION_CAPTURE_INTERVAL = 60000; // 60 seconds in milliseconds
    QString m_savePath = "../captures"; // Default save path

    cv::VideoCapture *m_capture;
    QTimer *m_timer;
    QElapsedTimer m_fpsTimer;
    int m_frameCount = 0;
    double m_currentFps = 0.0;
    MotionDetector *m_motionDetector;
    bool m_cameraOpened = false;
};

#endif // CV_WEBCAM_CAPTURE_H
