#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cv_webcam_capture.h"  // Добавлен заголовочный файл
#include <QPainter>
#include <QMessageBox>  // Добавлен заголовочный файл

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_webcam(new CVWebcamCapture(this))
{
    ui->setupUi(this);

    // Настройка главного окна
    this->resize(800, 600);
    this->setMinimumSize(640, 480);

    // Настройка области отображения видео
    ui->cameraLabel->setAlignment(Qt::AlignCenter);
    ui->cameraLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->cameraLabel->setStyleSheet("background-color: black;");

    // Настройка метки FPS
    ui->fpsLabel->setStyleSheet("background-color: rgba(0,0,0,50%); color: white; padding: 5px;");

    connect(m_webcam, &CVWebcamCapture::new_frame, this, &MainWindow::update_frame);
    connect(m_webcam, &CVWebcamCapture::camera_error, this, &MainWindow::handle_camera_error);

    if(!m_webcam->start_camera()) {
        handle_camera_error(tr("Failed to initialize camera. Please check if camera is connected."));
    }
}

MainWindow::~MainWindow()
{
    delete m_webcam;
    delete ui;
}

void MainWindow::update_frame(QImage frame, double fps, bool motionDetected)
{
    if(frame.isNull()) return;

    // Обновление FPS
    ui->fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));

    // Подготовка изображения
    QPixmap pixmap = QPixmap::fromImage(frame);
    if(pixmap.isNull()) return;

    // Масштабирование с сохранением пропорций
    pixmap = pixmap.scaled(ui->cameraLabel->size(),
                           Qt::KeepAspectRatio,
                           Qt::SmoothTransformation);

    // Создание центрированного изображения
    QPixmap centeredPixmap(ui->cameraLabel->size());
    centeredPixmap.fill(Qt::black);
    QPainter painter(&centeredPixmap);
    painter.drawPixmap((ui->cameraLabel->width() - pixmap.width())/2,
                       (ui->cameraLabel->height() - pixmap.height())/2,
                       pixmap);

    ui->cameraLabel->setPixmap(centeredPixmap);

    // Обновление статуса движения
    ui->motionLabel->setText(motionDetected ? "Motion Detected: Yes" : "Motion Detected: No");
    ui->cameraLabel->setStyleSheet(QString("background-color: black; border: 2px solid %1;")
                                       .arg(motionDetected ? "red" : "green"));
}

void MainWindow::handle_camera_error(const QString &message)
{
    ui->cameraLabel->setText(message);
    ui->cameraLabel->setStyleSheet("color: red; font-weight: bold; font-size: 16px; background-color: black;");

    if(QMessageBox::question(this, tr("Camera Error"),
                              tr("%1\nTry to reconnect?").arg(message)) == QMessageBox::Yes) {
        if(m_webcam->start_camera()) {  // Исправлено на правильное имя метода
            ui->cameraLabel->setText("");
            ui->cameraLabel->setStyleSheet("background-color: black;");
        }
    }
}
