#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>  // Добавлен заголовочный файл для QMessageBox

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CVWebcamCapture;  // Предварительное объявление

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void update_frame(QImage frame, double fps, bool motionDetected);
    void handle_camera_error(const QString &message);

private:
    Ui::MainWindow *ui;
    CVWebcamCapture *m_webcam;
};

#endif // MAINWINDOW_H
