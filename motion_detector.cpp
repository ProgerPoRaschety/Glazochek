#include "motion_detector.h"
#include <QDebug>

MotionDetector::MotionDetector() :
    m_firstFrame(true),
    m_sensitivityLevel(2),
    m_threshold(25),
    m_minContourArea(500)
{}

void MotionDetector::setSensitivity(int level) {
    m_sensitivityLevel = level;
    switch (level) {
    case 0: m_threshold = 40; m_minContourArea = 2000; break;
    case 1: m_threshold = 35; m_minContourArea = 1500; break;
    case 2: m_threshold = 25; m_minContourArea = 500; break;
    case 3: m_threshold = 15; m_minContourArea = 200; break;
    case 4: m_threshold = 10; m_minContourArea = 100; break;
    case 5: m_threshold = 5; m_minContourArea = 50; break;
    }
}

bool MotionDetector::detectMotion(const cv::Mat& frame, cv::Mat& outputFrame, double& motionPercentage) {
    if (frame.empty()) {
        qDebug() << "Empty frame received in motion detection";
        return false;
    }

    try {
        cv::cvtColor(frame, m_grayFrame, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(m_grayFrame, m_grayFrame, cv::Size(21, 21), 0);

        if (m_firstFrame) {
            m_previousFrame = m_grayFrame.clone();
            m_firstFrame = false;
            return false;
        }

        cv::absdiff(m_previousFrame, m_grayFrame, m_diffFrame);
        cv::threshold(m_diffFrame, m_threshFrame, m_threshold, 255, cv::THRESH_BINARY);
        cv::dilate(m_threshFrame, m_threshFrame, cv::Mat(), cv::Point(-1, -1), 2);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(m_threshFrame, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        bool motionDetected = false;
        double totalFrameArea = frame.rows * frame.cols;
        double motionArea = 0;

        for (const auto& contour : contours) {
            if (cv::contourArea(contour) < m_minContourArea) continue;

            motionDetected = true;
            cv::Rect boundingRect = cv::boundingRect(contour);
            cv::rectangle(outputFrame, boundingRect, cv::Scalar(0, 255, 0), 2);
            motionArea += boundingRect.area();
        }

        m_previousFrame = m_grayFrame.clone();
        motionPercentage = (motionArea / totalFrameArea) * 100.0;

        return motionDetected;

    } catch (const cv::Exception& e) {
        qCritical() << "Motion detection error:" << e.what();
        return false;
    }
}
