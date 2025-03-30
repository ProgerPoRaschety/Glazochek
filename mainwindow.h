#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QResizeEvent>  // Добавляем необходимый заголовочный файл

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CVWebcamCapture;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;  // Добавляем объявление

private slots:
    void update_frame(QImage frame, double fps, bool motionDetected);
    void handle_camera_error(const QString &message);
    void on_sensitivitySlider_valueChanged(int value);

private:
    Ui::MainWindow *ui;
    CVWebcamCapture *m_webcam;
    QImage m_currentFrame;
    double m_currentFps = 0.0;
    bool m_lastMotionState = false;
};

#endif // MAINWINDOW_H
