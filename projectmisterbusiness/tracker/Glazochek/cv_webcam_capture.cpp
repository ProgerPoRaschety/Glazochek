#include "cv_webcam_capture.h"
#include <QDebug>
#include <QDateTime>    // <-- Добавлено для логгера
#include <QCoreApplication> // <-- Добавлено (опционально, для пути)

CVWebcamCapture::CVWebcamCapture(QObject *parent)
    : QObject(parent),
    m_capture(nullptr),
    m_timer(new QTimer(this)),
    m_motionDetector(new MotionDetector()),
    m_cameraOpened(false)
// m_logFileName = QCoreApplication::applicationDirPath() + "/motion_log.txt"; // <-- Опционально: задать путь
{
    connect(m_timer, &QTimer::timeout, this, &CVWebcamCapture::process_frame);
    m_fpsTimer.start();
}

CVWebcamCapture::~CVWebcamCapture()
{
    stop_camera(); // Останавливает таймер и камеру
    closeLogger(); // <-- Закрываем логгер
    delete m_timer; // Уже было
    delete m_motionDetector; // Уже было
}

// --- Методы Логирования (Добавлены) ---
bool CVWebcamCapture::initializeLogger() {
    closeLogger(); // Закрываем, если был открыт
    m_logFile.setFileName(m_logFileName);
    // Открываем файл как обычно
    if (!m_logFile.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Failed to open log file:" << m_logFile.errorString();
        return false;
    }
    m_logStream.setDevice(&m_logFile);
    // m_logStream.setCodec("UTF-8"); // <-- ЗАКОММЕНТИРУЙТЕ ИЛИ УДАЛИТЕ ЭТУ СТРОКУ
    qInfo() << "Logger initialized. Writing to:" << m_logFileName;
    // Добавляем заголовок, если файл пустой
    if (m_logFile.size() == 0) {
        m_logStream << "Timestamp, Coverage (%)" << Qt::endl;
        m_logStream.flush();
    }
    return true;
}

void CVWebcamCapture::logMotionDetected(double coveragePercentage) {
    if (!m_logFile.isOpen()) {
        qWarning() << "Attempting to write to closed log file.";
        // Попытка переоткрыть (может быть не лучшей идеей в реальном времени)
        // if (!initializeLogger()) { return; }
        return; // Просто не пишем, если закрыт
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logEntry = QString("%1, %2")
                           .arg(timestamp)
                           .arg(QString::number(coveragePercentage, 'f', 2)); // Формат с 2 знаками после запятой

    m_logStream << logEntry << Qt::endl; // endl вызывает flush
    m_logStream.flush(); // Дополнительный явный flush для надежности
}

void CVWebcamCapture::closeLogger() {
    if (m_logFile.isOpen()) {
        qInfo() << "Closing log file:" << m_logFileName;
        // Убедимся, что все записано
        m_logStream.flush();
        m_logFile.close();
        // Отсоединяем QTextStream от файла
        m_logStream.setDevice(nullptr);
    }
}
// --- Конец методов Логирования ---


bool CVWebcamCapture::start_camera(int camera_index)
{
    stop_camera(); // Останавливаем предыдущую, если была

    try {
        m_capture = new cv::VideoCapture(camera_index);

        if (!m_capture || !m_capture->isOpened()) {
            emit camera_error(tr("Could not open camera (index %1)").arg(camera_index));
            qCritical() << "Failed to open camera index:" << camera_index;
            delete m_capture;
            m_capture = nullptr;
            return false;
        }

        // Настройки камеры (можно вынести)
        m_capture->set(cv::CAP_PROP_FRAME_WIDTH, 640);
        m_capture->set(cv::CAP_PROP_FRAME_HEIGHT, 480);
        m_capture->set(cv::CAP_PROP_FPS, 30);

        // Инициализируем логгер ПОСЛЕ успешного открытия камеры
        if (!initializeLogger()) {
            qWarning() << "Logger initialization failed, proceeding without logging.";
            // Решите, является ли это критической ошибкой
        }

        m_timer->start(33); // ~30 FPS
        m_frameCount = 0;
        m_fpsTimer.restart();
        m_cameraOpened = true;
        qInfo() << "Camera" << camera_index << "started successfully.";
        return true;

    } catch (const cv::Exception& e) {
        emit camera_error(tr("OpenCV exception during camera start: %1").arg(e.what()));
        qCritical() << "OpenCV exception during camera start:" << e.what();
        // Убедимся, что ресурсы освобождены в случае исключения
        delete m_capture;
        m_capture = nullptr;
        m_cameraOpened = false;
        return false;
    }
}

void CVWebcamCapture::stop_camera()
{
    if (m_timer->isActive()) {
        m_timer->stop();
        qInfo() << "Camera timer stopped.";
    }

    // Закрываем логгер перед освобождением камеры
    closeLogger(); // <-- Закрываем логгер здесь

    if (m_capture && m_capture->isOpened()) {
        qInfo() << "Releasing camera capture...";
        m_capture->release();
        qInfo() << "Camera capture released.";
    }

    delete m_capture;
    m_capture = nullptr;
    m_cameraOpened = false;
    qInfo() << "Camera resources cleaned up.";
}

void CVWebcamCapture::setSensitivity(int level)
{
    if (m_motionDetector) {
        m_motionDetector->setSensitivity(level);
    }
}

void CVWebcamCapture::process_frame()
{
    if (!m_cameraOpened || !m_capture || !m_capture->isOpened()) {
        return; // Нечего обрабатывать
    }

    cv::Mat frame;
    if (!m_capture->read(frame) || frame.empty()) {
        qWarning() << "Failed to read frame from camera";
        emit camera_error(tr("Camera disconnected or failed to read frame"));
        stop_camera(); // Останавливаем все при ошибке чтения
        return;
    }

    // Расчет FPS (как было)
    m_frameCount++;
    if (m_fpsTimer.elapsed() >= 1000) {
        m_currentFps = m_frameCount * 1000.0 / m_fpsTimer.elapsed();
        m_frameCount = 0;
        m_fpsTimer.restart();
    }

    cv::Mat outputFrame; // Кадр для отрисовки контуров
    double coveragePercentage = 0.0; // Процент покрытия от детектора
    bool motionDetected = false; // Флаг для сигнала UI

    try {
        outputFrame = frame.clone(); // Клонируем исходный кадр для рисования
        // Вызываем детектор, получаем процент покрытия
        coveragePercentage = m_motionDetector->detectMotion(frame, outputFrame);

        // Определяем флаг движения на основе процента (можно добавить порог > 0)
        motionDetected = (coveragePercentage > 0.0);

        // --- Логирование ---
        if (motionDetected) {
            logMotionDetected(coveragePercentage); // Логируем, если есть движение
        }
        // --- Конец Логирования ---

        // Конвертируем кадр с нарисованными прямоугольниками в RGB для Qt
        cv::cvtColor(outputFrame, outputFrame, cv::COLOR_BGR2RGB);

    } catch (const cv::Exception& e) {
        qCritical() << "OpenCV processing error in process_frame:" << e.what();
        // В случае ошибки, отправим исходный кадр без изменений
        cv::cvtColor(frame, outputFrame, cv::COLOR_BGR2RGB); // Конвертируем оригинал
        motionDetected = false; // Считаем, что движения не было
        // Логировать ошибку?
    }

    // Создаем QImage (как было)
    QImage image(outputFrame.data,
                 outputFrame.cols,
                 outputFrame.rows,
                 static_cast<int>(outputFrame.step), // Используем static_cast
                 QImage::Format_RGB888);

    // Отправляем сигнал (как было, но с флагом motionDetected)
    if (!image.isNull()) {
        // Отправляем копию QImage и флаг motionDetected
        emit new_frame(image.copy(), m_currentFps, motionDetected);
    } else {
        qWarning() << "Created QImage is null.";
    }
}
