#include "PlaybackQueue.hpp"
#include <random>
#include <algorithm>

namespace soniq::audio {

PlaybackQueue::PlaybackQueue() = default;

void PlaybackQueue::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_items.clear();
    m_shuffleIndices.clear();
    m_currentIndex = 0;
    m_shufflePosition = 0;
}

uint64_t PlaybackQueue::append(QueueItem item) {
    std::lock_guard<std::mutex> lock(m_mutex);
    item.uid = m_nextUid++;
    m_items.push_back(std::move(item));

    if (m_shuffle) {
        generateShuffleMap();
    }
    return m_items.back().uid;
}

uint64_t PlaybackQueue::insertNext(QueueItem item) {
    std::lock_guard<std::mutex> lock(m_mutex);
    item.uid = m_nextUid++;

    size_t insertPos = (m_items.empty()) ? 0 : m_currentIndex + 1;
    if (insertPos > m_items.size()) insertPos = m_items.size();

    m_items.insert(m_items.begin() + insertPos, std::move(item));

    if (m_shuffle) {
        generateShuffleMap();
    }
    return m_items[insertPos].uid;
}

void PlaybackQueue::insert(size_t index, QueueItem item) {
    std::lock_guard<std::mutex> lock(m_mutex);
    item.uid = m_nextUid++;
    if (index > m_items.size()) index = m_items.size();
    m_items.insert(m_items.begin() + index, std::move(item));
    if (m_shuffle) {
        generateShuffleMap();
    }
}

bool PlaybackQueue::remove(size_t index) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (index >= m_items.size()) return false;

    m_items.erase(m_items.begin() + index);

    if (m_items.empty()) {
        m_currentIndex = 0;
    } else if (m_currentIndex >= m_items.size()) {
        m_currentIndex = m_items.size() - 1;
    }

    if (m_shuffle) {
        generateShuffleMap();
    }
    return true;
}

bool PlaybackQueue::removeByUid(uint64_t uid) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find_if(m_items.begin(), m_items.end(), [uid](const QueueItem& q) {
        return q.uid == uid;
    });
    if (it == m_items.end()) return false;
    m_items.erase(it);

    if (m_items.empty()) {
        m_currentIndex = 0;
    } else if (m_currentIndex >= m_items.size()) {
        m_currentIndex = m_items.size() - 1;
    }

    if (m_shuffle) {
        generateShuffleMap();
    }
    return true;
}

bool PlaybackQueue::move(size_t fromIndex, size_t toIndex) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (fromIndex >= m_items.size() || toIndex >= m_items.size() || fromIndex == toIndex) {
        return false;
    }

    QueueItem item = std::move(m_items[fromIndex]);
    m_items.erase(m_items.begin() + fromIndex);
    m_items.insert(m_items.begin() + toIndex, std::move(item));

    if (m_currentIndex == fromIndex) {
        m_currentIndex = toIndex;
    } else if (fromIndex < m_currentIndex && toIndex >= m_currentIndex) {
        m_currentIndex--;
    } else if (fromIndex > m_currentIndex && toIndex <= m_currentIndex) {
        m_currentIndex++;
    }

    if (m_shuffle) {
        generateShuffleMap();
    }
    return true;
}

void PlaybackQueue::setQueue(std::vector<QueueItem> items, size_t startIndex) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_items = std::move(items);
    for (auto& item : m_items) {
        if (item.uid == 0) item.uid = m_nextUid++;
    }
    m_currentIndex = (startIndex < m_items.size()) ? startIndex : 0;
    if (m_shuffle) {
        generateShuffleMap();
    }
}

std::vector<QueueItem> PlaybackQueue::items() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_items;
}

size_t PlaybackQueue::size() const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_items.size();
}

bool PlaybackQueue::empty() const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_items.empty();
}

std::optional<QueueItem> PlaybackQueue::currentItem() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_items.empty() || m_currentIndex >= m_items.size()) {
        return std::nullopt;
    }
    return m_items[m_currentIndex];
}

bool PlaybackQueue::setCurrentIndex(size_t index) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (index >= m_items.size()) return false;
    m_currentIndex = index;
    return true;
}

bool PlaybackQueue::hasNext() const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_items.empty()) return false;
    if (m_repeatMode == core::RepeatMode::Track || m_repeatMode == core::RepeatMode::Queue) {
        return true;
    }
    if (m_shuffle) {
        return m_shufflePosition + 1 < m_shuffleIndices.size();
    }
    return m_currentIndex + 1 < m_items.size();
}

bool PlaybackQueue::hasPrevious() const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_items.empty()) return false;
    if (m_repeatMode == core::RepeatMode::Track || m_repeatMode == core::RepeatMode::Queue) {
        return true;
    }
    if (m_shuffle) {
        return m_shufflePosition > 0;
    }
    return m_currentIndex > 0;
}

std::optional<QueueItem> PlaybackQueue::next() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_items.empty()) return std::nullopt;

    if (m_repeatMode == core::RepeatMode::Track) {
        return m_items[m_currentIndex];
    }

    if (m_shuffle) {
        if (m_shufflePosition + 1 < m_shuffleIndices.size()) {
            m_shufflePosition++;
            m_currentIndex = m_shuffleIndices[m_shufflePosition];
            return m_items[m_currentIndex];
        } else if (m_repeatMode == core::RepeatMode::Queue) {
            generateShuffleMap();
            m_shufflePosition = 0;
            m_currentIndex = m_shuffleIndices[0];
            return m_items[m_currentIndex];
        }
        return std::nullopt;
    }

    if (m_currentIndex + 1 < m_items.size()) {
        m_currentIndex++;
        return m_items[m_currentIndex];
    } else if (m_repeatMode == core::RepeatMode::Queue) {
        m_currentIndex = 0;
        return m_items[0];
    }

    return std::nullopt;
}

std::optional<QueueItem> PlaybackQueue::previous() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_items.empty()) return std::nullopt;

    if (m_repeatMode == core::RepeatMode::Track) {
        return m_items[m_currentIndex];
    }

    if (m_shuffle) {
        if (m_shufflePosition > 0) {
            m_shufflePosition--;
            m_currentIndex = m_shuffleIndices[m_shufflePosition];
            return m_items[m_currentIndex];
        }
        return m_items[m_currentIndex];
    }

    if (m_currentIndex > 0) {
        m_currentIndex--;
        return m_items[m_currentIndex];
    } else if (m_repeatMode == core::RepeatMode::Queue) {
        m_currentIndex = m_items.size() - 1;
        return m_items[m_currentIndex];
    }

    return m_items[0];
}

void PlaybackQueue::setShuffle(bool enabled) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_shuffle = enabled;
    if (m_shuffle) {
        generateShuffleMap();
    }
}

void PlaybackQueue::generateShuffleMap() {
    m_shuffleIndices.resize(m_items.size());
    for (size_t i = 0; i < m_items.size(); ++i) {
        m_shuffleIndices[i] = i;
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_shuffleIndices.begin(), m_shuffleIndices.end(), g);

    // Make sure current track stays at current position in shuffle list
    if (!m_items.empty()) {
        auto it = std::find(m_shuffleIndices.begin(), m_shuffleIndices.end(), m_currentIndex);
        if (it != m_shuffleIndices.end()) {
            std::iter_swap(m_shuffleIndices.begin(), it);
        }
    }
    m_shufflePosition = 0;
}

} // namespace soniq::audio
