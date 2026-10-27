#include "LibraryScanner.hpp"
#include "metadata/AudioProbe.hpp"
#include "metadata/ArtworkExtractor.hpp"
#include "core/Logger.hpp"
#include <filesystem>
#include <algorithm>
#include <QUrl>
#include <QDir>
#include <QFileInfo>

namespace soniq::library {

LibraryScanner::LibraryScanner(QObject* parent)
    : QObject(parent) {}

LibraryScanner::~LibraryScanner() {
    cancel();
}

void LibraryScanner::cancel() {
    m_cancelRequested = true;
    if (m_workerThread && m_workerThread->joinable()) {
        m_workerThread->join();
    }
    if (m_scanning) {
        m_scanning = false;
        emit scanningChanged();
    }
}

bool LibraryScanner::replaceOnScan() const {
    return db::DatabaseManager::instance().getPreference("replace_on_scan", "1") != "0";
}

void LibraryScanner::setReplaceOnScan(bool replace) {
    db::DatabaseManager::instance().setPreference("replace_on_scan", replace ? "1" : "0");
    emit replaceOnScanChanged();
}

void LibraryScanner::clearLibrary() {
    cancel();
    SONIQ_LOG_INFO("Manual library clear executed.");
    db::DatabaseManager::instance().clearAllTracks();
    emit scanFinished(0, 0);
}

void LibraryScanner::scanDirectory(const QString& directoryPath, bool replaceExisting) {
    if (m_scanning) {
        SONIQ_LOG_WARN("Scan already in progress, ignoring duplicate request.");
        return;
    }

    QString cleanPath = directoryPath.trimmed();
    QUrl url(cleanPath);
    if (url.isValid() && (url.isLocalFile() || url.scheme() == "file")) {
        cleanPath = url.toLocalFile();
    } else {
        if (cleanPath.startsWith("file:///")) cleanPath = cleanPath.mid(8);
        else if (cleanPath.startsWith("file://")) cleanPath = cleanPath.mid(7);
        cleanPath = QUrl::fromPercentEncoding(cleanPath.toUtf8());
    }

    cleanPath = QDir::cleanPath(cleanPath);
    if (cleanPath.startsWith('/') && cleanPath.length() >= 3 && cleanPath[2] == ':') {
        cleanPath = cleanPath.mid(1);
    }
    cleanPath = QDir::toNativeSeparators(cleanPath);

    QFileInfo fi(cleanPath);
    if (!fi.exists() || !fi.isDir()) {
        SONIQ_LOG_ERROR("Target scan directory does not exist or is not a directory: {}", cleanPath.toStdString());
        return;
    }

    SONIQ_LOG_INFO("Starting library scan for directory: {}", cleanPath.toStdString());

    cancel();
    m_scanning = true;
    m_cancelRequested = false;
    m_currentCount = 0;
    m_totalCount = 0;
    m_currentFile.clear();
    emit scanningChanged();
    emit scanProgressChanged();

    bool doReplace = replaceExisting && replaceOnScan();
    m_workerThread = std::make_unique<std::thread>(&LibraryScanner::doScan, this, cleanPath, doReplace);
}

void LibraryScanner::doScan(const QString& path, bool replaceExisting) {
    emit scanStarted(path);

    namespace fs = std::filesystem;
    fs::path scanFsPath(path.toStdWString());
    std::vector<fs::path> audioFiles;

    std::error_code ec;
    if (fs::exists(scanFsPath, ec) && fs::is_directory(scanFsPath, ec)) {
        for (const auto& entry : fs::recursive_directory_iterator(scanFsPath, fs::directory_options::skip_permission_denied, ec)) {
            if (m_cancelRequested) break;
            if (entry.is_regular_file(ec)) {
                std::string ext = entry.path().extension().string();
                if (metadata::AudioProbe::isFormatSupported(ext)) {
                    audioFiles.push_back(entry.path());
                }
            }
        }
    } else {
        SONIQ_LOG_ERROR("Scan target directory does not exist or is not a directory: {}", path.toStdString());
    }

    if (replaceExisting) {
        SONIQ_LOG_INFO("Single folder mode: replacing existing library with folder {}", path.toStdString());
        db::DatabaseManager::instance().clearAllTracks();
    }

    const int total = static_cast<int>(audioFiles.size());
    m_totalCount = total;
    int addedCount = 0;
    SONIQ_LOG_INFO("Discovered {} potential audio files to scan in: {}", total, path.toStdString());
    emit scanProgressChanged();

    for (int i = 0; i < total; ++i) {
        if (m_cancelRequested) break;

        m_currentCount = i + 1;
        QString qFilePath = QString::fromStdWString(audioFiles[i].wstring());
        std::string filePath = qFilePath.toStdString();
        m_currentFile = qFilePath;

        emit scanProgress(i + 1, total, qFilePath);
        emit scanProgressChanged();

        auto inspection = metadata::AudioProbe::probeFile(filePath);
        if (inspection.success) {
            db::TrackRecord rec;
            rec.filePath = filePath;
            rec.title = inspection.metadata.title;
            rec.artist = inspection.metadata.artist;
            rec.album = inspection.metadata.album;
            rec.albumArtist = inspection.metadata.albumArtist;
            rec.genre = inspection.metadata.genre;
            rec.year = inspection.metadata.year;
            rec.trackNumber = inspection.metadata.trackNumber;
            rec.discNumber = inspection.metadata.discNumber;
            rec.durationSeconds = inspection.metadata.durationSeconds;
            rec.sampleRate = inspection.format.sampleRate();
            rec.bitDepth = inspection.format.bitDepth();
            rec.channels = inspection.format.channels();
            rec.bitrate = inspection.format.bitrate();
            rec.format = inspection.format.codec();
            rec.fileSize = inspection.metadata.fileSize;
            rec.artworkPath = metadata::ArtworkExtractor::instance().getOrExtractArtwork(filePath);

            if (db::DatabaseManager::instance().insertOrUpdateTrack(rec) > 0) {
                addedCount++;
            }
        }
    }

    if (!m_cancelRequested && addedCount > 0) {
        db::DatabaseManager::instance().setPreference("last_scanned_folder", path.toStdString());
    }

    m_scanning = false;
    emit scanningChanged();
    emit scanFinished(addedCount, total);
    SONIQ_LOG_INFO("Scan finished: added {} of {} audio files.", addedCount, total);
}

} // namespace soniq::library
