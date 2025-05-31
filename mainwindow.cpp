#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QMessageBox>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QTextStream>
#include <QTimer>
#include <QFileInfo>
#include <QPushButton>
#include <QFont>
#include <QDebug>
QString MainWindow::findLatestLogFile() const
{
    QDir dir(m_webcam->getSavePath());
    QStringList logFiles = dir.entryList({"motion_log_*.txt"}, QDir::Files, QDir::Time);
    return logFiles.isEmpty() ? QString() : dir.filePath(logFiles.first());
}

void JournalDialog::loadLogContent()
{
    textEdit->clear();
    QFile file(currentLogPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        textEdit->setText(in.readAll());
        file.close();
        textEdit->moveCursor(QTextCursor::End);
        textEdit->append("\n\nFile path: " + currentLogPath);
    } else {
        textEdit->setText("Could not open log file:\n" + currentLogPath +
                          "\nError: " + file.errorString());
    }
}

JournalDialog::JournalDialog(const QString& logPath, QWidget *parent)
    : QDialog(parent), currentLogPath(logPath)
{
    setWindowTitle("Event Journal - " + QFileInfo(logPath).fileName());
    resize(800, 600);

    textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Courier New", 10));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(textEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout();

    QPushButton *refreshBtn = new QPushButton("Refresh", this);
    refreshBtn->setStyleSheet("padding: 5px;");
    connect(refreshBtn, &QPushButton::clicked, this, &JournalDialog::loadLogContent);

    QPushButton *closeBtn = new QPushButton("Close", this);
    closeBtn->setStyleSheet("padding: 5px;");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);

    btnLayout->addWidget(refreshBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    setLayout(layout);
    loadLogContent();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_webcam(new CVWebcamCapture(this))
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);

    this->resize(800, 600);
    this->setMinimumSize(640, 480);

    ui->cameraLabel->setAlignment(Qt::AlignCenter);
    ui->cameraLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->cameraLabel->setStyleSheet("background-color: black;");

    ui->fpsLabel->setStyleSheet("background-color: rgba(0,0,0,50%); color: white; padding: 5px;");
    ui->motionLabel->setStyleSheet("background-color: rgba(0,0,0,50%); color: white; padding: 5px;");

    ui->startButton->setText("Start");
    setButtonStartStyle();

    connect(m_webcam, &CVWebcamCapture::new_frame, this, &MainWindow::update_frame);
    connect(m_webcam, &CVWebcamCapture::camera_error, this, &MainWindow::handle_camera_error);
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::on_startButton_clicked);

    setupCloseButton();
    connect(ui->closeButton, &QPushButton::clicked, this, &MainWindow::on_closeButton_clicked);

    setDarkTheme();
    setSensitivity(2);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_webcam;
    if (m_journalDialog) {
        m_journalDialog->deleteLater();
    }
}
void MainWindow::positionCloseButton()
{
    ui->closeButton->setFixedSize(30, 30);

    int margin = 100;
    ui->closeButton->move(this->width() - ui->closeButton->width() - margin, margin);
    ui->closeButton->raise();

    connect(this, &MainWindow::resizeEvent, [=](QResizeEvent*) {
        ui->closeButton->move(this->width() - ui->closeButton->width() - margin, margin);
    });
}

void MainWindow::setupCloseButton()
{
    ui->closeButton->setText("X");
    ui->closeButton->setFixedSize(30, 30);
    ui->closeButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #ff5c5c;"
        "   color: white;"
        "   border-radius: 15px;"
        "   border: none;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #ff3b3b;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #ff1a1a;"
        "}"
    );

    QMenu* closeMenu = new QMenu(this);
    closeMenu->setStyleSheet(
        "QMenu {"
        "   background-color: #ff5c5c;"
        "   color: white;"
        "   border: 1px solid white;"
        "}"
        "QMenu::item:selected {"
        "   background-color: #ff3b3b;"
        "}"
    );

    QAction* exitAction = new QAction("Close Application", this);
    closeMenu->addAction(exitAction);
    connect(exitAction, &QAction::triggered, this, &MainWindow::on_closeButton_clicked);

    ui->closeButton->move(this->width() - ui->closeButton->width() - 10, 10);
    ui->closeButton->raise();


    connect(this, &MainWindow::resizeEvent, [=](QResizeEvent*) {
        ui->closeButton->move(this->width() - ui->closeButton->width() - 10, 10);
    });
}
void MainWindow::on_closeButton_clicked()
{
    this->close();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (!m_currentFrame.isNull()) {
        update_frame(m_currentFrame, m_currentFps, m_lastMotionState, m_lastMotionPercentage);
    }
    ui->closeButton->move(this->width() - ui->closeButton->width() - 20, ui->menubar->height()-22);
}

void MainWindow::update_frame(const QImage& frame, double fps, bool motionDetected, double motionPercentage)
{
    if (frame.isNull()) return;

    m_currentFrame = frame;
    m_currentFps = fps;
    m_lastMotionState = motionDetected;
    m_lastMotionPercentage = motionPercentage;

    ui->fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));
    ui->motionLabel->setText(QString("Motion: %1 (%2%)")
                                 .arg(motionDetected ? "Yes" : "No")
                                 .arg(motionPercentage, 0, 'f', 1));

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

void MainWindow::on_startButton_clicked()
{


    if (ui->startButton->text() == "Start") {
        ui->startButton->setText("Stop");
        setButtonStopStyle();

        if (!m_webcam->start_camera()) {
            handle_camera_error(tr("Failed to start camera."));
            ui->startButton->setText("Start");
            setButtonStartStyle();
        }
    } else {
        ui->startButton->setText("Start");
        setButtonStartStyle();
        m_webcam->stop_camera();
        clearCameraDisplay();
    }

}


void MainWindow::clearCameraDisplay()
{
    QPixmap blackPixmap(ui->cameraLabel->size());
    blackPixmap.fill(Qt::black);
    ui->cameraLabel->setPixmap(blackPixmap);
    ui->fpsLabel->clear();
    ui->motionLabel->clear();
    ui->cameraLabel->setStyleSheet("background-color: black; border: none;");
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About Webcam Viewer",
                       "<h2>Webcam Viewer 480p</h2>"
                       "<p>Version 1.0</p>"
                       "<p>A simple application for viewing webcam feed with motion detection.</p>");
}

void MainWindow::on_actionPreferences_triggered()
{
    QMenu settingsMenu;

    QMenu* themeMenu = settingsMenu.addMenu("Theme");
    themeMenu->addAction("Dark Theme", this, &MainWindow::setDarkTheme);
    themeMenu->addAction("Light Theme", this, &MainWindow::setLightTheme);

    QMenu* sensitivityMenu = settingsMenu.addMenu("Sensitivity");
    sensitivityMenu->addAction("Very Low", [this]() { setSensitivity(0); });
    sensitivityMenu->addAction("Low", [this]() { setSensitivity(1); });
    sensitivityMenu->addAction("Medium", [this]() { setSensitivity(2); });
    sensitivityMenu->addAction("High", [this]() { setSensitivity(3); });
    sensitivityMenu->addAction("Very High", [this]() { setSensitivity(4); });
    sensitivityMenu->addAction("Maximum", [this]() { setSensitivity(5); });

    settingsMenu.exec(QCursor::pos());
}

void MainWindow::on_actionJournal_triggered()
{
    QString logFile = findLatestLogFile();
    if (logFile.isEmpty()) {
        QMessageBox::information(this, "Journal",
                                 "No log files found in:\n" + m_webcam->getSavePath() +
                                     "\nMotion detection logs will appear here after first motion event.");
        return;
    }

    if (!m_journalDialog) {
        m_journalDialog = new JournalDialog(logFile, this);
        connect(m_journalDialog, &QDialog::finished, this, [this]() {
            m_journalDialog->deleteLater();
            m_journalDialog = nullptr;
        });
    } else {
        m_journalDialog->currentLogPath = logFile;
        m_journalDialog->loadLogContent();
    }

    m_journalDialog->show();
    m_journalDialog->raise();
    m_journalDialog->activateWindow();
}

void MainWindow::on_actionFolder_triggered()
{
    if (!m_folderOpened) {
        m_folderOpened = true;
        QString path = m_webcam->getSavePath();
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        QTimer::singleShot(1000, this, [this]() { m_folderOpened = false; });
    }
}

void MainWindow::setButtonStartStyle()
{
    ui->startButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   border: none;"
        "   padding: 5px;"
        "   border-radius: 4px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #45a049;"
        "}"
        );
}

void MainWindow::setButtonStopStyle()
{
    ui->startButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #f44336;"
        "   color: white;"
        "   border: none;"
        "   padding: 5px;"
        "   border-radius: 4px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #d32f2f;"
        "}"
        );
}

void MainWindow::setDarkTheme()
{
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42,130,218));
    darkPalette.setColor(QPalette::Highlight, QColor(42,130,218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    qApp->setPalette(darkPalette);

    this->setStyleSheet("QMainWindow { background-color: #353535; }");
    ui->cameraLabel->setStyleSheet("background-color: black;");
    ui->fpsLabel->setStyleSheet("background-color: rgba(0,0,0,50%); color: white; padding: 5px;");
    ui->motionLabel->setStyleSheet("background-color: rgba(0,0,0,50%); color: white; padding: 5px;");
}

void MainWindow::setLightTheme()
{
    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::WindowText, QColor(50, 50, 50));
    lightPalette.setColor(QPalette::Base, Qt::white);
    lightPalette.setColor(QPalette::AlternateBase, QColor(220, 220, 220));
    lightPalette.setColor(QPalette::ToolTipBase, Qt::white);
    lightPalette.setColor(QPalette::ToolTipText, QColor(50, 50, 50));
    lightPalette.setColor(QPalette::Text, QColor(50, 50, 50));
    lightPalette.setColor(QPalette::Button, QColor(230, 230, 230));
    lightPalette.setColor(QPalette::ButtonText, QColor(50, 50, 50));
    lightPalette.setColor(QPalette::BrightText, Qt::red);
    lightPalette.setColor(QPalette::Link, QColor(0, 0, 200));
    lightPalette.setColor(QPalette::Highlight, QColor(0, 0, 200));
    lightPalette.setColor(QPalette::HighlightedText, Qt::white);

    qApp->setPalette(lightPalette);

    this->setStyleSheet("QMainWindow { background-color: #f0f0f0; }");
    ui->cameraLabel->setStyleSheet("background-color: #f0f0f0;");
    ui->fpsLabel->setStyleSheet("background-color: rgba(240,240,240,50%); color: #323232; padding: 5px;");
    ui->motionLabel->setStyleSheet("background-color: rgba(240,240,240,50%); color: #323232; padding: 5px;");
}

void MainWindow::setSensitivity(int level)
{
    QString sensitivityText;
    switch (level) {
    case 0: sensitivityText = "Very Low"; break;
    case 1: sensitivityText = "Low"; break;
    case 2: sensitivityText = "Medium"; break;
    case 3: sensitivityText = "High"; break;
    case 4: sensitivityText = "Very High"; break;
    case 5: sensitivityText = "Maximum"; break;
    }
    ui->sensitivityLabel->setText("Sensitivity: " + sensitivityText);
    m_webcam->setSensitivity(level);
}
