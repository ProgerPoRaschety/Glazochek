#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cv_webcam_capture.h"  // Убедитесь, что этот заголовочный файл включен
#include <QPainter>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_webcam(new CVWebcamCapture(this))
{
    ui->setupUi(this);

    this->resize(800, 600);
    this->setMinimumSize(640, 480);

    ui->cameraLabel->setAlignment(Qt::AlignCenter);
    ui->cameraLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->cameraLabel->setStyleSheet("background-color: black;");

    ui->fpsLabel->setStyleSheet("background-color: rgba(0,0,0,50%); color: white; padding: 5px;");

    ui->sensitivitySlider->setRange(0, 5);
    ui->sensitivitySlider->setValue(2);
    ui->sensitivitySlider->setTickInterval(1);
    ui->sensitivitySlider->setTickPosition(QSlider::TicksBelow);
    ui->sensitivityLabel->setText("Sensitivity: Medium");

    connect(m_webcam, &CVWebcamCapture::new_frame, this, &MainWindow::update_frame);
    connect(m_webcam, &CVWebcamCapture::camera_error, this, &MainWindow::handle_camera_error);
    connect(ui->sensitivitySlider, &QSlider::valueChanged, this, &MainWindow::on_sensitivitySlider_valueChanged);

    if (!m_webcam->start_camera()) {
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
    if (frame.isNull()) return;

    m_currentFrame = frame;
    m_currentFps = fps;
    m_lastMotionState = motionDetected;

    ui->fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));

    QPixmap pixmap = QPixmap::fromImage(frame);
    if (pixmap.isNull()) return;

    pixmap = pixmap.scaled(ui->cameraLabel->size(),
                           Qt::KeepAspectRatio,
                           Qt::SmoothTransformation);

    QPixmap centeredPixmap(ui->cameraLabel->size());
    centeredPixmap.fill(Qt::black);
    QPainter painter(&centeredPixmap);
    painter.drawPixmap((ui->cameraLabel->width() - pixmap.width()) / 2,
                       (ui->cameraLabel->height() - pixmap.height()) / 2,
                       pixmap);

    ui->cameraLabel->setPixmap(centeredPixmap);
    ui->motionLabel->setText(motionDetected ? "Motion Detected: Yes" : "Motion Detected: No");
    ui->cameraLabel->setStyleSheet(QString("background-color: black; border: 2px solid %1;")
                                       .arg(motionDetected ? "red" : "green"));
}

void MainWindow::handle_camera_error(const QString &message)
{
    ui->cameraLabel->setText(message);
    ui->cameraLabel->setStyleSheet("color: red; font-weight: bold; font-size: 16px; background-color: black;");

    if (QMessageBox::question(this, tr("Camera Error"),
                              tr("%1\nTry to reconnect?").arg(message)) == QMessageBox::Yes) {
        if (m_webcam->start_camera()) {
            ui->cameraLabel->setText("");
            ui->cameraLabel->setStyleSheet("background-color: black;");
        }
    }
}

void MainWindow::on_sensitivitySlider_valueChanged(int value)
{
    QString sensitivityText;
    switch (value) {
    case 0: sensitivityText = "Very Low"; break;
    case 1: sensitivityText = "Low"; break;
    case 2: sensitivityText = "Medium"; break;
    case 3: sensitivityText = "High"; break;
    case 4: sensitivityText = "Very High"; break;
    case 5: sensitivityText = "Maximum"; break;
    }
    ui->sensitivityLabel->setText("Sensitivity: " + sensitivityText);
    m_webcam->setSensitivity(value);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (!m_currentFrame.isNull()) {
        update_frame(m_currentFrame, m_currentFps, m_lastMotionState);
    }
}
