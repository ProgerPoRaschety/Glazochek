#ifndef MOTION_DETECTOR_H
#define MOTION_DETECTOR_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector> // Добавлено для std::vector

class MotionDetector
{
public:
    MotionDetector();
    // Изменено: возвращает double (процент покрытия), 0.0 если нет движения
    double detectMotion(const cv::Mat& frame, cv::Mat& outputFrame);
    void setSensitivity(int level);
    double getLastCoveragePercentage() const; // Новый метод для получения последнего процента

private:
    cv::Mat m_previousFrame;
    cv::Mat m_grayFrame;
    cv::Mat m_diffFrame;
    cv::Mat m_threshFrame;
    bool m_firstFrame;
    int m_sensitivityLevel;
    int m_threshold;
    int m_minContourArea;
    double m_lastCoveragePercentage = 0.0; // Новая переменная для хранения процента
};

#endif
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QResizeEvent>
#include "cv_webcam_capture.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CVWebcamCapture;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;
private slots:
    void update_frame(QImage frame, double fps, bool motionDetected);
    void handle_camera_error(const QString &message);
    void on_sensitivitySlider_valueChanged(int value);

private:
    Ui::MainWindow *ui;
    CVWebcamCapture *m_webcam;
    QImage m_currentFrame;
    double m_currentFps = 0.0;
    bool m_lastMotionState = false;
};

#endif
