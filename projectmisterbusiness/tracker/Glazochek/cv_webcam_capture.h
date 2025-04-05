#ifndef CV_WEBCAM_CAPTURE_H
#define CV_WEBCAM_CAPTURE_H

#include <QObject>
#include <QTimer>
#include <QImage>
#include <QElapsedTimer>
#include <QFile>         // <-- Добавлено
#include <QTextStream>   // <-- Добавлено
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include "motion_detector.h" // Уже было

class CVWebcamCapture : public QObject
{
    Q_OBJECT
public:
    explicit CVWebcamCapture(QObject *parent = nullptr);
    ~CVWebcamCapture(); // Деструктор уже был

    bool start_camera(int camera_index = 0);
    void stop_camera();
    void setSensitivity(int level);

signals:
    // Сигнал остается прежним, передаем bool для удобства UI
    void new_frame(QImage frame, double fps, bool motionDetected);
    void camera_error(QString message);

private slots:
    void process_frame();

private:
    // Логирование
    QFile m_logFile;
    QTextStream m_logStream;
    QString m_logFileName = "motion_log.txt"; // Имя файла журнала
    bool initializeLogger();
    void logMotionDetected(double coveragePercentage); // Принимает процент
    void closeLogger();

    // Камера и детектор
    cv::VideoCapture *m_capture;
    QTimer *m_timer;
    QElapsedTimer m_fpsTimer;
    int m_frameCount = 0;
    double m_currentFps = 0.0;
    MotionDetector *m_motionDetector; // Уже было
    bool m_cameraOpened = false; // Уже было
};

#endif // CV_WEBCAM_CAPTURE_H
