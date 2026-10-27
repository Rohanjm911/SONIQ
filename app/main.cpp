#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QQuickStyle>
#include <QFile>
#include <QDir>
#include <QQuickWindow>
#include <QScreen>

#include "core/Logger.hpp"
#include "database/DatabaseManager.hpp"
#include "audio/AudioPlaybackEngine.hpp"
#include "services/PlaybackController.hpp"
#include "services/LibraryModel.hpp"
#include "services/AudioInspectorService.hpp"
#include "services/DspService.hpp"
#include "services/SettingsService.hpp"
#include "library/LibraryScanner.hpp"

int main(int argc, char* argv[]) {
    // 1. Initialize High-DPI and Qt Attributes
    QGuiApplication app(argc, argv);
    app.setApplicationName("SONIQ");
    app.setOrganizationName("SONIQ");
    app.setApplicationVersion("1.0.0");

    QQuickStyle::setStyle("Basic");

    // 2. Initialize Structured Logger
    soniq::core::Logger::instance().setLevel(soniq::core::LogLevel::Debug);
    SONIQ_LOG_INFO("==================================================");
    SONIQ_LOG_INFO("Starting SONIQ v1.0.0 — Pure Sound. Infinite Possibilities.");
    SONIQ_LOG_INFO("==================================================");

    // 3. Initialize SQLite Database
    if (!soniq::db::DatabaseManager::instance().initialize()) {
        SONIQ_LOG_ERROR("Fatal: Failed to initialize SQLite database.");
    }

    // 4. Instantiate Core Audio Playback Engine
    soniq::audio::AudioPlaybackEngine engine;

    // 5. Instantiate Services
    soniq::services::PlaybackController playbackController(engine, &app);
    soniq::services::LibraryModel libraryModel(&app);
    soniq::services::AudioInspectorService audioInspector(&app);
    soniq::services::DspService dspService(engine, &app);
    soniq::services::SettingsService settingsService(&app);
    soniq::library::LibraryScanner libraryScanner(&app);

    // Connect library scanner finished signal to refresh library model
    QObject::connect(&libraryScanner, &soniq::library::LibraryScanner::scanFinished,
                     &libraryModel, &soniq::services::LibraryModel::refresh);

    // 6. Setup QML Engine
    QQmlApplicationEngine qmlEngine;
    qmlEngine.rootContext()->setContextProperty("playbackController", &playbackController);
    qmlEngine.rootContext()->setContextProperty("libraryModel", &libraryModel);
    qmlEngine.rootContext()->setContextProperty("audioInspector", &audioInspector);
    qmlEngine.rootContext()->setContextProperty("dspService", &dspService);
    qmlEngine.rootContext()->setContextProperty("settingsService", &settingsService);
    qmlEngine.rootContext()->setContextProperty("libraryScanner", &libraryScanner);

    const QUrl url(QStringLiteral("qrc:/ui/main.qml"));
    QObject::connect(&qmlEngine, &QQmlApplicationEngine::warnings, [](const QList<QQmlError>& errors) {
        for (const auto& err : errors) {
            SONIQ_LOG_ERROR("QML Diagnostic Error: {}", err.toString().toStdString());
        }
    });

    QObject::connect(&qmlEngine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject* obj, const QUrl& objUrl) {
        if (!obj && url == objUrl) {
            SONIQ_LOG_CRIT("Failed to load root QML interface from QRC.");
        }
    }, Qt::QueuedConnection);

    qmlEngine.addImportPath("qrc:/");
    qmlEngine.addImportPath("qrc:/ui");

    // Primary: Load from compiled QRC resources
    qmlEngine.load(url);

    // Fallbacks for local file development
    if (qmlEngine.rootObjects().isEmpty()) {
        QString p1 = app.applicationDirPath() + "/../ui/main.qml";
        if (QFile::exists(p1)) {
            qmlEngine.addImportPath(app.applicationDirPath() + "/..");
            qmlEngine.load(QUrl::fromLocalFile(p1));
        }
    }
    if (qmlEngine.rootObjects().isEmpty()) {
        QString p2 = app.applicationDirPath() + "/ui/main.qml";
        if (QFile::exists(p2)) {
            qmlEngine.addImportPath(app.applicationDirPath());
            qmlEngine.load(QUrl::fromLocalFile(p2));
        }
    }
    if (qmlEngine.rootObjects().isEmpty()) {
        QString p3 = QDir::currentPath() + "/ui/main.qml";
        if (QFile::exists(p3)) {
            qmlEngine.addImportPath(QDir::currentPath());
            qmlEngine.load(QUrl::fromLocalFile(p3));
        }
    }

    if (qmlEngine.rootObjects().isEmpty()) {
        SONIQ_LOG_CRIT("Fatal: Could not load main.qml component.");
        return -1;
    }

    if (auto* rootObj = qmlEngine.rootObjects().value(0, nullptr)) {
        if (auto* win = qobject_cast<QQuickWindow*>(rootObj)) {
            QScreen* screen = win->screen() ? win->screen() : QGuiApplication::primaryScreen();
            if (screen) {
                const QRect avail = screen->availableGeometry();
                int w = std::min(1200, std::max(840, static_cast<int>(avail.width() * 0.82)));
                int h = std::min(720, std::max(520, static_cast<int>(avail.height() * 0.82)));
                int x = avail.x() + (avail.width() - w) / 2;
                int y = avail.y() + (avail.height() - h) / 2;
                win->setGeometry(x, y, w, h);
                SONIQ_LOG_INFO("Window positioned at ({}, {}) with dimensions {}x{} (Monitor Work Area: {}x{})",
                               x, y, w, h, avail.width(), avail.height());
            }
        }
    }

    SONIQ_LOG_INFO("UI initialized successfully. Entering main event loop.");
    return app.exec();
}
