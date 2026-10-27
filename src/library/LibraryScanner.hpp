#pragma once

#include "database/DatabaseManager.hpp"
#include <QObject>
#include <QString>
#include <QStringList>
#include <atomic>
#include <thread>
#include <memory>

namespace soniq::library {

class LibraryScanner : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isScanning READ isScanning NOTIFY scanningChanged)
    Q_PROPERTY(int scannedCount READ scannedCount NOTIFY scanProgressChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY scanProgressChanged)
    Q_PROPERTY(QString currentScanningFile READ currentScanningFile NOTIFY scanProgressChanged)
    Q_PROPERTY(bool replaceOnScan READ replaceOnScan WRITE setReplaceOnScan NOTIFY replaceOnScanChanged)

public:
    explicit LibraryScanner(QObject* parent = nullptr);
    ~LibraryScanner() override;

    [[nodiscard]] bool isScanning() const noexcept { return m_scanning; }
    [[nodiscard]] int scannedCount() const noexcept { return m_currentCount; }
    [[nodiscard]] int totalCount() const noexcept { return m_totalCount; }
    [[nodiscard]] QString currentScanningFile() const { return m_currentFile; }
    [[nodiscard]] bool replaceOnScan() const;
    void setReplaceOnScan(bool replace);

public slots:
    void scanDirectory(const QString& directoryPath, bool replaceExisting = true);
    void clearLibrary();
    void cancel();

signals:
    void scanningChanged();
    void scanProgressChanged();
    void replaceOnScanChanged();
    void scanStarted(const QString& path);
    void scanProgress(int current, int total, const QString& currentFile);
    void scanFinished(int addedCount, int totalFound);

private:
    void doScan(const QString& path, bool replaceExisting);

    std::atomic<bool> m_scanning{false};
    std::atomic<bool> m_cancelRequested{false};
    std::atomic<int> m_currentCount{0};
    std::atomic<int> m_totalCount{0};
    QString m_currentFile;
    std::unique_ptr<std::thread> m_workerThread;
};

} // namespace soniq::library
