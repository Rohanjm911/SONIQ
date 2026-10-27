#pragma once

#include "metadata/AudioProbe.hpp"
#include <QObject>
#include <QString>
#include <QVariantMap>

namespace soniq::services {

class AudioInspectorService : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString filePath READ filePath NOTIFY inspected)
    Q_PROPERTY(QString title READ title NOTIFY inspected)
    Q_PROPERTY(QString artist READ artist NOTIFY inspected)
    Q_PROPERTY(QString album READ album NOTIFY inspected)
    Q_PROPERTY(QString containerFormat READ containerFormat NOTIFY inspected)
    Q_PROPERTY(QString codecLongName READ codecLongName NOTIFY inspected)
    Q_PROPERTY(int sampleRate READ sampleRate NOTIFY inspected)
    Q_PROPERTY(int bitDepth READ bitDepth NOTIFY inspected)
    Q_PROPERTY(int channels READ channels NOTIFY inspected)
    Q_PROPERTY(int bitrate READ bitrate NOTIFY inspected)
    Q_PROPERTY(double durationSeconds READ durationSeconds NOTIFY inspected)
    Q_PROPERTY(qulonglong fileSize READ fileSize NOTIFY inspected)
    Q_PROPERTY(bool isLossless READ isLossless NOTIFY inspected)
    Q_PROPERTY(bool isHiRes READ isHiRes NOTIFY inspected)
    Q_PROPERTY(bool isBitPerfectEligible READ isBitPerfectEligible NOTIFY inspected)
    Q_PROPERTY(QString bitPerfectAssessment READ bitPerfectAssessment NOTIFY inspected)
    Q_PROPERTY(QString technicalSummary READ technicalSummary NOTIFY inspected)

public:
    explicit AudioInspectorService(QObject* parent = nullptr);

    [[nodiscard]] QString filePath() const { return QString::fromStdString(m_result.filePath); }
    [[nodiscard]] QString title() const { return QString::fromStdString(m_result.metadata.title); }
    [[nodiscard]] QString artist() const { return QString::fromStdString(m_result.metadata.artist); }
    [[nodiscard]] QString album() const { return QString::fromStdString(m_result.metadata.album); }
    [[nodiscard]] QString containerFormat() const { return QString::fromStdString(m_result.containerFormat); }
    [[nodiscard]] QString codecLongName() const { return QString::fromStdString(m_result.codecLongName); }
    [[nodiscard]] int sampleRate() const { return static_cast<int>(m_result.format.sampleRate()); }
    [[nodiscard]] int bitDepth() const { return m_result.format.bitDepth(); }
    [[nodiscard]] int channels() const { return m_result.format.channels(); }
    [[nodiscard]] int bitrate() const { return static_cast<int>(m_result.bitRate / 1000); }
    [[nodiscard]] double durationSeconds() const { return m_result.metadata.durationSeconds; }
    [[nodiscard]] qulonglong fileSize() const { return m_result.metadata.fileSize; }
    [[nodiscard]] bool isLossless() const { return m_result.isLossless; }
    [[nodiscard]] bool isHiRes() const { return m_result.isHiRes; }
    [[nodiscard]] bool isBitPerfectEligible() const { return m_result.isBitPerfectEligible; }
    [[nodiscard]] QString bitPerfectAssessment() const { return QString::fromStdString(m_result.bitPerfectAssessment); }
    [[nodiscard]] QString technicalSummary() const { return QString::fromStdString(m_result.technicalSummary); }

public slots:
    void inspectFile(const QString& path);

signals:
    void inspected();

private:
    metadata::AudioInspectionResult m_result;
};

} // namespace soniq::services
