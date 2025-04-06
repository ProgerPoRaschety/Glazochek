#include "motion_detector.h"
#include <QDebug>

MotionDetector::MotionDetector() :
    m_firstFrame(true),
    m_sensitivityLevel(2),
    m_threshold(25),
    m_minContourArea(500)
{}

void MotionDetector::setSensitivity(int level)
{
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

bool MotionDetector::detectMotion(const cv::Mat& frame, cv::Mat& outputFrame)
{
    if (frame.empty()) {
        qDebug() << "Empty frame received in motion detection";
        return false;
    }

    try {
        cv::cvtColor(frame, m_grayFrame, cv::COLOR_BGR2GRAY);
        //Конвертирует BGR (стандарт OpenCV) в оттенки серого для упрощения анализа
        cv::GaussianBlur(m_grayFrame, m_grayFrame, cv::Size(21, 21), 0);
        //Применяет размытие Гаусса (ядро 21x21) для уменьшения шумов
        if (m_firstFrame) {
            m_previousFrame = m_grayFrame.clone();
            m_firstFrame = false;
            qDebug() << "First frame captured for motion detection";
            return false;
        }

        cv::absdiff(m_previousFrame, m_grayFrame, m_diffFrame);
        //Вычисляет абсолютную разницу между текущим и предыдущим кадром
        cv::threshold(m_diffFrame, m_threshFrame, m_threshold, 255, cv::THRESH_BINARY);
        //Преобразует разницу в чёрно-белое изображение, где белые пиксели - зоны изменений
        cv::dilate(m_threshFrame, m_threshFrame, cv::Mat(), cv::Point(-1, -1), 2);
        //Расширяет (увеличивает) белые области для объединения близких изменений

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(m_threshFrame, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        //Находит границы всех белых областей (зон движения)

        bool motionDetected = false;
        for (const auto& contour : contours) {
            if (cv::contourArea(contour) < m_minContourArea) continue;

            motionDetected = true;
            cv::rectangle(outputFrame, cv::boundingRect(contour), cv::Scalar(0, 255, 0), 2);
        }

        m_previousFrame = m_grayFrame.clone();

        if (motionDetected) {
            qDebug() << "Motion detected with sensitivity level:" << m_sensitivityLevel;
        }

        return motionDetected;
        //Отбрасывает мелкие контуры и рисует прямоугольники вокруг значимых движений
    } catch (const cv::Exception& e) {
        qCritical() << "Motion detection error:" << e.what();
        return false;
    }
}
