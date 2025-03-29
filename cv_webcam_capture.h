#ifndef CV_WEBCAM_CAPTURE_H
#define CV_WEBCAM_CAPTURE_H

#include <QObject>
#include <QTimer>
#include <QImage>
#include <QElapsedTimer>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

class CVWebcamCapture : public QObject
{
    Q_OBJECT
public:
    explicit CVWebcamCapture(QObject *parent = nullptr);
    virtual ~CVWebcamCapture();

    bool start_camera(int camera_index = 0);
    void stop_camera();

signals:
    void new_frame(QImage frame, double fps);
    void camera_error(QString message);

private slots:
    void process_frame();

private:
    cv::VideoCapture *m_capture;
    QTimer *m_timer;
    cv::Mat m_frame;
    QElapsedTimer m_fpsTimer;
    int m_frameCount = 0;
    double m_currentFps = 0.0;
};

#endif // CV_WEBCAM_CAPTURE_H
