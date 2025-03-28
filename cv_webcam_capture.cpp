#include "cv_webcam_capture.h"
#include <QDebug>

CVWebcamCapture::CVWebcamCapture(QObject *parent)
    : QObject(parent),
    m_capture(nullptr),
    m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &CVWebcamCapture::process_frame);
}

CVWebcamCapture::~CVWebcamCapture()
{
    stop_camera();
    delete m_timer;
}

bool CVWebcamCapture::start_camera(int camera_index)
{
    if(m_capture) {
        stop_camera();
    }

    m_capture = new cv::VideoCapture(camera_index);
    if(!m_capture->isOpened()) {
        emit camera_error("Could not open camera");
        delete m_capture;
        m_capture = nullptr;
        return false;
    }

    m_timer->start(36);
    return true;
}

void CVWebcamCapture::stop_camera()
{
    if(m_timer->isActive()) {
        m_timer->stop();
    }
    if(m_capture) {
        if(m_capture->isOpened()) {
            m_capture->release();
        }
        delete m_capture;
        m_capture = nullptr;
    }
}

void CVWebcamCapture::process_frame()
{
    if(!m_capture || !m_capture->isOpened()) {
        return;
    }

    *m_capture >> m_frame;
    if(m_frame.empty()) {
        return;
    }

    cv::cvtColor(m_frame, m_frame, cv::COLOR_BGR2RGB);
    QImage image(m_frame.data,
                 m_frame.cols,
                 m_frame.rows,
                 m_frame.step,
                 QImage::Format_RGB888);

    emit new_frame(image.copy());
}
