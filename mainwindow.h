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

    // Метод для обновления FPS в интерфейсе
    void updateFpsDisplay(double fps);

private slots:
    void update_frame(QImage frame, double fps);
    // Слот для обработки ошибок камеры
    void handle_camera_error(const QString &message);

private:
    Ui::MainWindow *ui;

    // Указатель на класс работы с веб-камерой
    CVWebcamCapture *m_webcam;

    // QLabel для отображения видео с камеры
    QLabel *m_cameraLabel;

    // QLabel для отображения FPS
    QLabel *m_fpsLabel;

    // Метод для инициализации интерфейса
    void initUI();

    // Метод для настройки соединений сигналов и слотов
    void setupConnections();
};

#endif // MAINWINDOW_H
