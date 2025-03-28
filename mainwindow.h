#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
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

private slots:
    void update_frame(const QImage &frame);
    void handle_camera_error(const QString &message);

private:
    Ui::MainWindow *ui;
    CVWebcamCapture *m_webcam;
    QLabel *m_cameraLabel;
};
#endif // MAINWINDOW_H
