#include "motion_detector.h"
#include <QDebug>
#include <vector> // Убедитесь, что vector включен

MotionDetector::MotionDetector() :
    m_firstFrame(true),
    m_sensitivityLevel(2), // Medium по умолчанию
    m_threshold(25),
    m_minContourArea(500),
    m_lastCoveragePercentage(0.0) // Инициализация
{
    setSensitivity(m_sensitivityLevel); // Устанавливаем начальные пороги
}

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
        m_threshold = 15; // <-- Исправлена возможная ошибка копипаста
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
    qDebug() << "Sensitivity set to level" << level << "Threshold:" << m_threshold << "Min Area:" << m_minContourArea;
}

// Изменено: возвращает процент покрытия
double MotionDetector::detectMotion(const cv::Mat& frame, cv::Mat& outputFrame)
{
    m_lastCoveragePercentage = 0.0; // Сбрасываем процент для текущего кадра

    if (frame.empty()) {
        qWarning() << "Empty frame received in motion detection";
        return m_lastCoveragePercentage; // Возвращаем 0.0
    }

    try {
        cv::cvtColor(frame, m_grayFrame, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(m_grayFrame, m_grayFrame, cv::Size(21, 21), 0);

        if (m_firstFrame) {
            m_previousFrame = m_grayFrame.clone();
            m_firstFrame = false;
            return m_lastCoveragePercentage; // Возвращаем 0.0 для первого кадра
        }

        cv::absdiff(m_previousFrame, m_grayFrame, m_diffFrame);
        cv::threshold(m_diffFrame, m_threshFrame, m_threshold, 255, cv::THRESH_BINARY);
        cv::dilate(m_threshFrame, m_threshFrame, cv::Mat(), cv::Point(-1, -1), 2);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(m_threshFrame.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // Используем clone для findContours, т.к. он может изменять входное изображение

        double totalMotionArea = 0.0;
        bool motionFoundThisFrame = false; // Флаг, что нашли хотя бы один контур

        for (const auto& contour : contours) {
            double area = cv::contourArea(contour);
            if (area < m_minContourArea) continue;

            motionFoundThisFrame = true;
            totalMotionArea += area; // Суммируем площадь значимых контуров
            cv::rectangle(outputFrame, cv::boundingRect(contour), cv::Scalar(0, 0, 255), 2); // Красный прямоугольник
        }

        m_previousFrame = m_grayFrame.clone(); // Обновляем предыдущий кадр

        // Вычисляем процент покрытия
        double frameArea = static_cast<double>(m_grayFrame.rows * m_grayFrame.cols);
        if (frameArea > 0 && totalMotionArea > 0) {
            m_lastCoveragePercentage = (totalMotionArea / frameArea) * 100.0;
        } else {
            m_lastCoveragePercentage = 0.0;
        }

        //if (motionFoundThisFrame) {
        //    qDebug() << "Motion detected. Coverage:" << m_lastCoveragePercentage << "%";
        //}

        return m_lastCoveragePercentage; // Возвращаем вычисленный процент

    } catch (const cv::Exception& e) {
        qCritical() << "Motion detection error:" << e.what();
        return 0.0; // Возвращаем 0.0 в случае ошибки
    }
}

double MotionDetector::getLastCoveragePercentage() const
{
    return m_lastCoveragePercentage;
}
