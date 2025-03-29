#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QDebug>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_webcam(new CVWebcamCapture(this))
{
    ui->setupUi(this);

    // Create and setup camera label
    m_cameraLabel = new QLabel(this);
    m_cameraLabel->setAlignment(Qt::AlignCenter);
    setCentralWidget(m_cameraLabel);

    // Create and setup FPS label
    m_fpsLabel = new QLabel(this);
    m_fpsLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);
    m_fpsLabel->setStyleSheet("QLabel { background-color : rgba(0,0,0,50%); color : white; padding: 5px; }");
    m_fpsLabel->setText("FPS: 0.0");
    m_fpsLabel->setFixedSize(100, 30);

    // Create layout and add widgets
    QVBoxLayout *layout = new QVBoxLayout(m_cameraLabel);
    layout->addWidget(m_fpsLabel, 0, Qt::AlignRight | Qt::AlignTop);
    layout->setContentsMargins(0, 0, 0, 0);
    m_cameraLabel->setLayout(layout);

    connect(m_webcam, &CVWebcamCapture::new_frame, this, &MainWindow::update_frame);
    connect(m_webcam, &CVWebcamCapture::camera_error, this, &MainWindow::handle_camera_error);

    if(!m_webcam->start_camera()) {
        handle_camera_error("Failed to start camera");
    }
}

MainWindow::~MainWindow()
{
    delete m_webcam;  // Освобождаем ресурсы камеры
    delete ui;        // Освобождаем UI
}

void MainWindow::update_frame(QImage frame, double fps)
{
    // Обновление FPS
    m_fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));

    // Отображение кадра
    QPixmap pixmap = QPixmap::fromImage(frame);
    pixmap = pixmap.scaled(m_cameraLabel->size(),
                           Qt::KeepAspectRatio,
                           Qt::SmoothTransformation);
    m_cameraLabel->setPixmap(pixmap);
}
void MainWindow::handle_camera_error(const QString &message)
{
    // Показываем сообщение об ошибке
    m_cameraLabel->setText(message);
    m_cameraLabel->setStyleSheet("QLabel { color : red; font-weight: bold; }");

    // Выводим ошибку в консоль (теперь будет работать)
    qCritical() << "Camera error:" << message;
}
