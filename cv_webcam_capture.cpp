#include "cv_webcam_capture.h"
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <opencv2/imgcodecs.hpp>
CVWebcamCapture::CVWebcamCapture(QObject *parent)
    : QObject(parent),
    m_capture(nullptr),
    m_timer(new QTimer(this)),
    m_motionDetector(new MotionDetector()),
    m_cameraOpened(false)
{
    connect(m_timer, &QTimer::timeout, this, &CVWebcamCapture::process_frame);
    m_fpsTimer.start();
}

CVWebcamCapture::~CVWebcamCapture()
{
    stop_camera();
    delete m_timer;
    delete m_motionDetector;
}

bool CVWebcamCapture::start_camera(int camera_index)
{
    stop_camera();

    try {
        m_capture = new cv::VideoCapture(camera_index);

        if (!m_capture->isOpened()) {
            emit camera_error(tr("Could not open camera (index %1)").arg(camera_index));
            delete m_capture;
            m_capture = nullptr;
            return false;
        }

        m_capture->set(cv::CAP_PROP_FRAME_WIDTH, 640);
        m_capture->set(cv::CAP_PROP_FRAME_HEIGHT, 480);
        m_capture->set(cv::CAP_PROP_FPS, 30);

        m_timer->start(33);
        m_frameCount = 0;
        m_fpsTimer.restart();
        m_cameraOpened = true;
        return true;

    } catch (const cv::Exception& e) {
        emit camera_error(tr("OpenCV exception: %1").arg(e.what()));
        return false;
    }
}

void CVWebcamCapture::stop_camera()
{
    if (m_timer->isActive()) {
        m_timer->stop();
    }

    if (m_capture && m_capture->isOpened()) {
        m_capture->release();
    }

    delete m_capture;
    m_capture = nullptr;
    m_cameraOpened = false;
}

void CVWebcamCapture::setSensitivity(int level)
{
    if (m_motionDetector) {
        m_motionDetector->setSensitivity(level);
    }
}

void CVWebcamCapture::process_frame()
{
    if (!m_cameraOpened || !m_capture || !m_capture->isOpened()) {
        return;
    }

    cv::Mat frame;
    if (!m_capture->read(frame) || frame.empty()) {
        qWarning() << "Failed to read frame from camera";
        emit camera_error(tr("Camera disconnected"));
        stop_camera();
        return;
    }

    m_frameCount++;
    if (m_fpsTimer.elapsed() >= 1000) {
        m_currentFps = m_frameCount * 1000.0 / m_fpsTimer.elapsed();
        m_frameCount = 0;
        m_fpsTimer.restart();
    }

    cv::Mat outputFrame;
    bool motionDetected = false;

    try {
        outputFrame = frame.clone();
        motionDetected = m_motionDetector->detectMotion(frame, outputFrame);

        // Photo capture logic
        if (motionDetected) {
            if (m_motionCaptureTimer.elapsed() >= MOTION_CAPTURE_INTERVAL) {
                // Capture photo
                QDir().mkpath(m_savePath);
                QString filename = QString("%1/motion_%2.jpg")
                                       .arg(m_savePath)
                                       .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz"));
                if (frame.empty()) { // Use original 'frame' for saving
                    qWarning() << "Frame is empty, cannot save image";
                } else {
                    cv::imwrite(filename.toStdString(), frame); // Save the original frame
                    qDebug() << "Captured motion photo:" << filename;
                }
                m_motionCaptureTimer.restart();
            }
            m_noMotionCaptureTimer.restart(); // Reset no motion timer if motion is detected
        } else {
            if (m_noMotionCaptureTimer.elapsed() >= NO_MOTION_CAPTURE_INTERVAL) {
                // Capture photo
                QDir().mkpath(m_savePath);
                QString filename = QString("%1/no_motion_%2.jpg")
                                       .arg(m_savePath)
                                       .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz"));
                if (frame.empty()) { // Use original 'frame' for saving
                    qWarning() << "Frame is empty, cannot save image";
                } else {
                    cv::imwrite(filename.toStdString(), frame); // Save the original frame
                    qDebug() << "Captured no motion photo:" << filename;
                }
                m_noMotionCaptureTimer.restart();
            }
            m_motionCaptureTimer.restart(); // Reset motion timer if no motion
        }

        cv::cvtColor(outputFrame, outputFrame, cv::COLOR_BGR2RGB);
    } catch (const cv::Exception& e) {
        qCritical() << "OpenCV processing error:" << e.what();
        return;
    }

    QImage image(outputFrame.data,
                 outputFrame.cols,
                 outputFrame.rows,
                 outputFrame.step,
                 QImage::Format_RGB888);

    if (!image.isNull()) {
        emit new_frame(image.copy(), m_currentFps, motionDetected);
    }
}
