#pragma once

#include "audio/AudioPlaybackEngine.hpp"
#include <QObject>
#include <QStringList>
#include <QVariantList>

namespace soniq::services {

class DspService : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY dspChanged)
    Q_PROPERTY(int mode READ mode WRITE setMode NOTIFY dspChanged)
    Q_PROPERTY(double preampDb READ preampDb WRITE setPreampDb NOTIFY dspChanged)
    Q_PROPERTY(QVariantList gains10 READ gains10 NOTIFY dspChanged)
    Q_PROPERTY(QVariantList gains31 READ gains31 NOTIFY dspChanged)
    Q_PROPERTY(bool crossfeedEnabled READ crossfeedEnabled WRITE setCrossfeedEnabled NOTIFY dspChanged)
    Q_PROPERTY(float stereoBalance READ stereoBalance WRITE setStereoBalance NOTIFY dspChanged)
    Q_PROPERTY(float stereoWidth READ stereoWidth WRITE setStereoWidth NOTIFY dspChanged)
    Q_PROPERTY(bool limiterEnabled READ limiterEnabled WRITE setLimiterEnabled NOTIFY dspChanged)
    Q_PROPERTY(QStringList presets READ presets CONSTANT)
    Q_PROPERTY(bool altersSignal READ altersSignal NOTIFY dspChanged)
    Q_PROPERTY(QStringList disablingReasons READ disablingReasons NOTIFY dspChanged)

public:
    explicit DspService(audio::AudioPlaybackEngine& engine, QObject* parent = nullptr);

    [[nodiscard]] bool isEnabled() const noexcept;
    void setEnabled(bool enable);

    [[nodiscard]] int mode() const noexcept;
    void setMode(int mode);

    [[nodiscard]] double preampDb() const noexcept;
    void setPreampDb(double db);

    [[nodiscard]] QVariantList gains10() const;
    [[nodiscard]] QVariantList gains31() const;

    [[nodiscard]] bool crossfeedEnabled() const noexcept;
    void setCrossfeedEnabled(bool enable);

    [[nodiscard]] float stereoBalance() const noexcept;
    void setStereoBalance(float balance);

    [[nodiscard]] float stereoWidth() const noexcept;
    void setStereoWidth(float width);

    [[nodiscard]] bool limiterEnabled() const noexcept;
    void setLimiterEnabled(bool enable);

    [[nodiscard]] QStringList presets() const;
    [[nodiscard]] bool altersSignal() const noexcept;
    [[nodiscard]] QStringList disablingReasons() const;

public slots:
    void setBandGain10(int band, double gainDb);
    void setBandGain31(int band, double gainDb);
    void applyPreset(const QString& name);
    void resetEq();

signals:
    void dspChanged();

private:
    audio::AudioPlaybackEngine& m_engine;
};

} // namespace soniq::services
