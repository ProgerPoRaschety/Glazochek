#ifndef MOTION_DETECTOR_H
#define MOTION_DETECTOR_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

class MotionDetector
{
public:
    MotionDetector();
    bool detectMotion(const cv::Mat& frame, cv::Mat& outputFrame);

private:
    cv::Mat m_previousFrame;
    cv::Mat m_grayFrame;
    cv::Mat m_diffFrame;
    cv::Mat m_threshFrame;
    bool m_firstFrame;
};

#endif // MOTION_DETECTOR_H
