#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QQuickStyle>

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
    QObject::connect(&qmlEngine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject* obj, const QUrl& objUrl) {
        if (!obj && url == objUrl) {
            SONIQ_LOG_CRIT("Failed to load root QML interface.");
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);

    // Fallback direct file load if resources not embedded
    qmlEngine.addImportPath("qrc:/");
    qmlEngine.load(QUrl::fromLocalFile(app.applicationDirPath() + "/../../ui/main.qml"));
    if (qmlEngine.rootObjects().isEmpty()) {
        qmlEngine.load(QUrl::fromLocalFile(app.applicationDirPath() + "/ui/main.qml"));
    }
    if (qmlEngine.rootObjects().isEmpty()) {
        qmlEngine.load(QUrl(QStringLiteral("d:/projects and certificates/projects/apps/SONIQ/ui/main.qml")));
    }

    SONIQ_LOG_INFO("UI initialized. Entering main event loop.");
    return app.exec();
}
