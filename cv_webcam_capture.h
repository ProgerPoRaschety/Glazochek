#ifndef CV_WEBCAM_CAPTURE_H
#define CV_WEBCAM_CAPTURE_H

#include <QObject>
#include <QTimer>
#include <QImage>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
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
    void setSavePath(const QString &path);
    QString getLogFilePath() const { return m_logFilePath; }
    QString getSavePath() const { return m_savePath; }

signals:
    void new_frame(QImage frame, double fps, bool motionDetected, double motionPercentage);
    void camera_error(QString message);

private slots:
    void process_frame();

private:
    // Изменён порядок объявления членов класса
    cv::VideoCapture *m_capture;
    QTimer *m_timer;
    MotionDetector *m_motionDetector;

    QElapsedTimer m_motionCaptureTimer;
    QElapsedTimer m_noMotionCaptureTimer;
    QElapsedTimer m_fpsTimer;

    QString m_savePath;
    QString m_logFilePath;
    QFile m_logFile;
    QTextStream m_logStream;

    int m_frameCount;
    double m_currentFps;
    bool m_cameraOpened;

    const int MOTION_CAPTURE_INTERVAL = 3000;
    const int NO_MOTION_CAPTURE_INTERVAL = 60000;

    void setupLogFile();
    void closeLogFile();
    void logMotion(double percentage);
};

#endif // CV_WEBCAM_CAPTURE_H
