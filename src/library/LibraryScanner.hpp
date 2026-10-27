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

public:
    explicit LibraryScanner(QObject* parent = nullptr);
    ~LibraryScanner() override;

    void scanDirectory(const QString& directoryPath);
    void cancel();

    [[nodiscard]] bool isScanning() const noexcept { return m_scanning; }

signals:
    void scanningChanged();
    void scanStarted(const QString& path);
    void scanProgress(int current, int total, const QString& currentFile);
    void scanFinished(int addedCount, int totalFound);

private:
    void doScan(const QString& path);

    std::atomic<bool> m_scanning{false};
    std::atomic<bool> m_cancelRequested{false};
    std::unique_ptr<std::thread> m_workerThread;
};

} // namespace soniq::library
