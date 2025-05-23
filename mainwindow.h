#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QStandardPaths>
#include <QDir>
#include <QTextEdit>
#include <QDialog>
#include "cv_webcam_capture.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class JournalDialog : public QDialog {
    Q_OBJECT
public:
    explicit JournalDialog(const QString& logPath, QWidget *parent = nullptr);
    void loadLogContent();
    QTextEdit *textEdit;
    QString currentLogPath;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void update_frame(const QImage& frame, double fps, bool motionDetected, double motionPercentage);
    void handle_camera_error(const QString &message);
    void on_pushButton_clicked();
    void clearCameraDisplay();
    void on_actionAbout_triggered();
    void on_actionPreferences_triggered();
    void on_actionJournal_triggered();
    void on_actionFolder_triggered();
    void on_closeButton_clicked();
    void setButtonStartStyle();
    void setButtonStopStyle();
    void setDarkTheme();
    void setLightTheme();
    void setSensitivity(int level);

private:
    Ui::MainWindow *ui;
    CVWebcamCapture *m_webcam;
    QImage m_currentFrame;
    double m_currentFps = 0.0;
    bool m_lastMotionState = false;
    double m_lastMotionPercentage = 0.0;
    QPoint m_dragPosition;
    JournalDialog *m_journalDialog = nullptr;
    bool m_folderOpened = false;

    QString findLatestLogFile() const;
    void setupCloseButton();
};

#endif // MAINWINDOW_H
