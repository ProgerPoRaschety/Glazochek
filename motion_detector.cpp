#include "motion_detector.h"
#include <QDebug>
#include <fstream>   // For logging
#include <chrono>    // For time
#include <iomanip>   // For formatting time
#include <ctime>

MotionDetector::MotionDetector() :
    m_firstFrame(true),
    m_sensitivityLevel(2),
    m_threshold(25),
    m_minContourArea(500)
{}

void MotionDetector::setSensitivity(int level) {
    m_sensitivityLevel = level;
    switch (level) {
    case 0: // Very Low
        m_threshold = 40;
        m_minContourArea = 2000;
        break;
    case 1: // Low
        m_threshold = 35;
        m_minContourArea = 1500;
        break;
    case 2: // Medium (default)
        m_threshold = 25;
        m_minContourArea = 500;
        break;
    case 3: // High
        m_threshold = 15;
        m_minContourArea = 200;
        break;
    case 4: // Very High
        m_threshold = 10;
        m_minContourArea = 100;
        break;
    case 5: // Maximum
        m_threshold = 5;
        m_minContourArea = 50;
        break;
    }
}

bool MotionDetector::detectMotion(const cv::Mat& frame, cv::Mat& outputFrame) {
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
            qDebug() << "First frame captured for motion detection";
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

        if (motionDetected) {
            qDebug() << "Motion detected with sensitivity level:" << m_sensitivityLevel;

            double coveragePercentage = (motionArea / totalFrameArea) * 100.0;

            // Get current time
            auto now = std::chrono::system_clock::now();
            std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
            std::tm* localTime = std::localtime(&currentTime);
            char timeString[100];
            std::strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", localTime);

            // Log to file
            std::ofstream logFile("motion_log.txt", std::ios::app);
            if (logFile.is_open()) {
                logFile << timeString << ", " << std::fixed << std::setprecision(2)
                << coveragePercentage << "%\n";
                logFile.close();
            } else {
                qWarning() << "Could not open log file for writing.";
            }
        }

        return motionDetected;

    } catch (const cv::Exception& e) {
        qCritical() << "Motion detection error:" << e.what();
        return false;
    }
}
