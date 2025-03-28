#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QPixmap>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_webcam(new CVWebcamCapture(this))
    , m_cameraLabel(new QLabel(this))
{
    ui->setupUi(this);

    // Настройка QLabel
    m_cameraLabel->setAlignment(Qt::AlignCenter);
    m_cameraLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setCentralWidget(m_cameraLabel);

    // Подключение сигналов
    connect(m_webcam, &CVWebcamCapture::new_frame,
            this, &MainWindow::update_frame);
    connect(m_webcam, &CVWebcamCapture::camera_error,
            this, &MainWindow::handle_camera_error);

    // Запуск камеры
    if(!m_webcam->start_camera()) {
        m_cameraLabel->setText("Ошибка инициализации камеры");
    }
}

void MainWindow::update_frame(const QImage &frame)
{
    QPixmap pixmap = QPixmap::fromImage(frame);
    m_cameraLabel->setPixmap(pixmap.scaled(
        m_cameraLabel->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));
}

void MainWindow::handle_camera_error(const QString &message)
{
    qWarning() << message;
    m_cameraLabel->setText(message);
    QMessageBox::critical(this, "Ошибка камеры", message);
    m_webcam->stop_camera();
}

MainWindow::~MainWindow()
{
    delete ui;
}
