// cv_webcam_capture.h
#ifndef CV_WEBCAM_CAPTURE_H
#define CV_WEBCAM_CAPTURE_H

#include <QObject>
#include <QTimer>
#include <QImage>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>
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
    void setSavePath(const QString &path); // Добавлено: Метод для установки пути сохранения

signals:
    void new_frame(QImage frame, double fps, bool motionDetected, double motionPercentage);
    void camera_error(QString message);

private slots:
    void process_frame();

private:
    QElapsedTimer m_motionCaptureTimer;
    QElapsedTimer m_noMotionCaptureTimer;
    QElapsedTimer m_fpsTimer;
    const int MOTION_CAPTURE_INTERVAL = 3000;
    const int NO_MOTION_CAPTURE_INTERVAL = 60000;
    QString m_savePath = "../captures"; // Оставляем по умолчанию, будет переопределено
    QFile m_logFile;
    QTextStream m_logStream;

    cv::VideoCapture *m_capture;
    QTimer *m_timer;
    int m_frameCount = 0;
    double m_currentFps = 0.0;
    MotionDetector *m_motionDetector;
    bool m_cameraOpened = false;

    void setupLogFile();
    void closeLogFile();
    void logMotion(double percentage);
};

#endif // CV_WEBCAM_CAPTURE_H
