#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QResizeEvent>
#include <QMouseEvent>
#include "cv_webcam_capture.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void update_frame(QImage frame, double fps, bool motionDetected);
    void handle_camera_error(const QString &message);
    void on_sensitivitySlider_valueChanged(int value);
    void on_pushButton_clicked();
    void clearCameraDisplay();
    void on_actionAbout_triggered();
    void on_actionPreferences_triggered();
    void on_closeButton_clicked();
    void setButtonStartStyle();
    void setButtonStopStyle();
    void setDarkTheme();
    void setLightTheme();

private:
    Ui::MainWindow *ui;
    CVWebcamCapture *m_webcam;
    QImage m_currentFrame;
    double m_currentFps = 0.0;
    bool m_lastMotionState = false;
    QPoint m_dragPosition;
    void setupCloseButton();
};

#endif // MAINWINDOW_H
