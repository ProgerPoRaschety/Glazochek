#ifndef CV_WEBCAM_CAPTURE_H
#define CV_WEBCAM_CAPTURE_H

#include <QObject>
#include <QTimer>
#include <QImage>
#include <opencv2/opencv.hpp>

class CVWebcamCapture : public QObject
{
    Q_OBJECT
public:
    explicit CVWebcamCapture(QObject *parent = nullptr);
    ~CVWebcamCapture();

    bool start_camera(int camera_index = 0);
    void stop_camera();

signals:
    void new_frame(QImage frame);
    void camera_error(QString message);

private slots:
    void process_frame();

private:
    cv::VideoCapture *m_capture;
    QTimer *m_timer;
    cv::Mat m_frame;
};

#endif // CV_WEBCAM_CAPTURE_H
