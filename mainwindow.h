#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>

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

private slots:
    void update_frame(QImage frame, double fps, bool motionDetected);
    void handle_camera_error(const QString &message);
    void on_sensitivitySlider_valueChanged(int value)
    {
        QString sensitivityText;
        switch(value) {
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

private:
    Ui::MainWindow *ui;
    CVWebcamCapture *m_webcam;
};

#endif // MAINWINDOW_H
