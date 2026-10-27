#include "AudioInspectorService.hpp"

namespace soniq::services {

AudioInspectorService::AudioInspectorService(QObject* parent)
    : QObject(parent) {}

void AudioInspectorService::inspectFile(const QString& path) {
    m_result = metadata::AudioProbe::probeFile(path.toStdString());
    emit inspected();
}

} // namespace soniq::services
