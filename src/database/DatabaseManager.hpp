#pragma once

#include "core/Types.hpp"
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <memory>
#include <vector>
#include <string>
#include <mutex>

namespace soniq::db {

struct TrackRecord {
    int64_t id{0};
    std::string filePath;
    std::string title;
    std::string artist;
    std::string album;
    std::string albumArtist;
    std::string genre;
    uint32_t year{0};
    uint32_t trackNumber{0};
    uint32_t discNumber{1};
    double durationSeconds{0.0};
    uint32_t sampleRate{44100};
    uint16_t bitDepth{16};
    uint16_t channels{2};
    uint32_t bitrate{0};
    std::string format;
    uint64_t fileSize{0};
    int64_t dateAdded{0};
    uint32_t playCount{0};
    int64_t lastPlayed{0};
    bool isFavorite{false};
    int rating{0};
    std::string artworkPath;
};

struct PlaylistRecord {
    int64_t id{0};
    std::string name;
    int64_t createdAt{0};
    int64_t updatedAt{0};
    bool isSmart{false};
    std::string rulesJson;
    size_t trackCount{0};
};

class DatabaseManager {
public:
    static DatabaseManager& instance();

    bool initialize(const QString& dbPath = "");
    void close();

    [[nodiscard]] bool isOpen() const;

    // Track CRUD operations
    int64_t insertOrUpdateTrack(const TrackRecord& track);
    bool removeTrack(int64_t trackId);
    bool removeTrackByPath(const std::string& filePath);
    bool clearAllTracks();
    std::vector<TrackRecord> getAllTracks(const std::string& sortBy = "title ASC");
    std::vector<TrackRecord> searchTracks(const std::string& query);
    std::vector<TrackRecord> getFavoriteTracks();
    std::vector<TrackRecord> getRecentlyPlayedTracks(size_t limit = 50);
    std::vector<TrackRecord> getMostPlayedTracks(size_t limit = 50);
    std::vector<TrackRecord> getRecentlyAddedTracks(size_t limit = 50);

    bool setTrackFavorite(int64_t trackId, bool favorite);
    bool incrementPlayCount(int64_t trackId);

    // Playlist operations
    int64_t createPlaylist(const std::string& name, bool isSmart = false, const std::string& rules = "");
    bool deletePlaylist(int64_t playlistId);
    bool renamePlaylist(int64_t playlistId, const std::string& newName);
    std::vector<PlaylistRecord> getAllPlaylists();
    bool addTrackToPlaylist(int64_t playlistId, int64_t trackId);
    bool removeTrackFromPlaylist(int64_t playlistId, int64_t trackId);
    std::vector<TrackRecord> getPlaylistTracks(int64_t playlistId);

    // Settings persistence
    void setPreference(const std::string& key, const std::string& value);
    [[nodiscard]] std::string getPreference(const std::string& key, const std::string& defaultValue = "");

    QSqlDatabase getDatabase() const;

private:
    DatabaseManager();
    ~DatabaseManager();

    bool runMigrations();

    QSqlDatabase m_db;
    QString m_connectionName;
    QString m_dbPath;
    Qt::HANDLE m_mainThreadId{nullptr};
    mutable std::mutex m_mutex;
};

} // namespace soniq::db
