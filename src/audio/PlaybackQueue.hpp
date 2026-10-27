#pragma once

#include "core/Types.hpp"
#include <vector>
#include <string>
#include <mutex>
#include <optional>
#include <cstdint>

namespace soniq::audio {

struct QueueItem {
    uint64_t uid{0};
    std::string filePath;
    std::string title;
    std::string artist;
    std::string album;
    double durationSeconds{0.0};
};

class PlaybackQueue {
public:
    PlaybackQueue();

    void clear();

    uint64_t append(QueueItem item);
    uint64_t insertNext(QueueItem item);
    void insert(size_t index, QueueItem item);
    bool remove(size_t index);
    bool removeByUid(uint64_t uid);
    bool move(size_t fromIndex, size_t toIndex);

    void setQueue(std::vector<QueueItem> items, size_t startIndex = 0);
    [[nodiscard]] std::vector<QueueItem> items() const;
    [[nodiscard]] size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

    // Current position
    [[nodiscard]] std::optional<QueueItem> currentItem() const;
    [[nodiscard]] size_t currentIndex() const noexcept { return m_currentIndex; }
    bool setCurrentIndex(size_t index);

    // Navigation
    [[nodiscard]] bool hasNext() const noexcept;
    [[nodiscard]] bool hasPrevious() const noexcept;
    std::optional<QueueItem> next();
    std::optional<QueueItem> previous();

    // Repeat and Shuffle
    void setRepeatMode(core::RepeatMode mode) noexcept { m_repeatMode = mode; }
    [[nodiscard]] core::RepeatMode repeatMode() const noexcept { return m_repeatMode; }

    void setShuffle(bool enabled);
    [[nodiscard]] bool isShuffle() const noexcept { return m_shuffle; }

private:
    void generateShuffleMap();

    mutable std::mutex m_mutex;
    std::vector<QueueItem> m_items;
    std::vector<size_t> m_shuffleIndices;
    size_t m_currentIndex{0};
    size_t m_shufflePosition{0};

    core::RepeatMode m_repeatMode{core::RepeatMode::Off};
    bool m_shuffle{false};
    uint64_t m_nextUid{1};
};

} // namespace soniq::audio
