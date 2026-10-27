#include "LibraryModel.hpp"
#include <algorithm>
#include <random>
#include <cctype>
#include <string_view>

namespace soniq::services {

LibraryModel::LibraryModel(QObject* parent)
    : QAbstractListModel(parent) {
    refresh();
}

int LibraryModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_tracks.size());
}

QVariant LibraryModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_tracks.size())) {
        return QVariant();
    }

    const auto& track = m_tracks[index.row()];
    switch (role) {
        case IdRole:                return static_cast<qint64>(track.id);
        case TitleRole:             return QString::fromStdString(track.title);
        case ArtistRole:            return QString::fromStdString(track.artist);
        case AlbumRole:             return QString::fromStdString(track.album);
        case GenreRole:             return QString::fromStdString(track.genre);
        case YearRole:              return track.year;
        case DurationRole:          return track.durationSeconds;
        case DurationFormattedRole: {
            int totalSec = static_cast<int>(track.durationSeconds);
            int mins = totalSec / 60;
            int secs = totalSec % 60;
            return QString::asprintf("%d:%02d", mins, secs);
        }
        case FilePathRole:          return QString::fromStdString(track.filePath);
        case FormatRole:            return QString::fromStdString(track.format);
        case FavoriteRole:          return track.isFavorite;
        case PlayCountRole:         return track.playCount;
        case ArtworkPathRole:       return QString::fromStdString(track.artworkPath);
        default:                    return QVariant();
    }
}

QHash<int, QByteArray> LibraryModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[ArtistRole] = "artist";
    roles[AlbumRole] = "album";
    roles[GenreRole] = "genre";
    roles[YearRole] = "year";
    roles[DurationRole] = "duration";
    roles[DurationFormattedRole] = "durationFormatted";
    roles[FilePathRole] = "filePath";
    roles[FormatRole] = "format";
    roles[FavoriteRole] = "isFavorite";
    roles[PlayCountRole] = "playCount";
    roles[ArtworkPathRole] = "artworkPath";
    return roles;
}

QString LibraryModel::sortLabel() const {
    if (m_sortField == "title") {
        return m_sortAscending ? "Title (A → Z)" : "Title (Z → A)";
    } else if (m_sortField == "artist") {
        return m_sortAscending ? "Artist (A → Z)" : "Artist (Z → A)";
    } else if (m_sortField == "album") {
        return m_sortAscending ? "Album (A → Z)" : "Album (Z → A)";
    } else if (m_sortField == "duration") {
        return m_sortAscending ? "Duration (Shortest)" : "Duration (Longest)";
    } else if (m_sortField == "date_added") {
        return m_sortAscending ? "Date Added (Oldest)" : "Date Added (Newest)";
    } else if (m_sortField == "format") {
        return m_sortAscending ? "Format (Lossless First)" : "Format (Lossy First)";
    } else if (m_sortField == "favorite") {
        return "Favorites First";
    } else if (m_sortField == "play_count") {
        return m_sortAscending ? "Least Played" : "Most Played";
    } else if (m_sortField == "track_number") {
        return m_sortAscending ? "Track Number (1 → N)" : "Track Number (N → 1)";
    } else if (m_sortField == "custom") {
        return "Shuffled";
    }
    return "Title (A → Z)";
}

void LibraryModel::applyCurrentSort() {
    if (m_sortField == "custom" || m_tracks.size() <= 1) {
        return;
    }

    auto compCaseInsensitive = [](std::string_view a, std::string_view b) {
        auto itA = a.begin();
        auto itB = b.begin();
        while (itA != a.end() && itB != b.end()) {
            char ca = static_cast<char>(std::tolower(static_cast<unsigned char>(*itA)));
            char cb = static_cast<char>(std::tolower(static_cast<unsigned char>(*itB)));
            if (ca != cb) return ca < cb;
            ++itA;
            ++itB;
        }
        return a.size() < b.size();
    };

    auto compEqualCaseInsensitive = [](std::string_view a, std::string_view b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (std::tolower(static_cast<unsigned char>(a[i])) !=
                std::tolower(static_cast<unsigned char>(b[i]))) {
                return false;
            }
        }
        return true;
    };

    std::stable_sort(m_tracks.begin(), m_tracks.end(), [&](const db::TrackRecord& a, const db::TrackRecord& b) {
        bool less = false;
        if (m_sortField == "title") {
            if (compEqualCaseInsensitive(a.title, b.title)) {
                less = compCaseInsensitive(a.artist, b.artist);
            } else {
                less = compCaseInsensitive(a.title, b.title);
            }
        } else if (m_sortField == "artist") {
            if (compEqualCaseInsensitive(a.artist, b.artist)) {
                if (compEqualCaseInsensitive(a.album, b.album)) {
                    less = a.trackNumber < b.trackNumber;
                } else {
                    less = compCaseInsensitive(a.album, b.album);
                }
            } else {
                less = compCaseInsensitive(a.artist, b.artist);
            }
        } else if (m_sortField == "album") {
            if (compEqualCaseInsensitive(a.album, b.album)) {
                if (a.discNumber != b.discNumber) {
                    less = a.discNumber < b.discNumber;
                } else {
                    less = a.trackNumber < b.trackNumber;
                }
            } else {
                less = compCaseInsensitive(a.album, b.album);
            }
        } else if (m_sortField == "duration") {
            if (a.durationSeconds == b.durationSeconds) {
                less = compCaseInsensitive(a.title, b.title);
            } else {
                less = a.durationSeconds < b.durationSeconds;
            }
        } else if (m_sortField == "date_added") {
            if (a.dateAdded == b.dateAdded) {
                less = a.id < b.id;
            } else {
                less = a.dateAdded < b.dateAdded;
            }
        } else if (m_sortField == "format") {
            auto getFormatPriority = [](const std::string& fmt) {
                std::string f = fmt;
                for (auto& c : f) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                if (f == "dsd" || f == "dsf" || f == "dff") return 1;
                if (f == "flac") return 2;
                if (f == "wav" || f == "wave") return 3;
                if (f == "alac" || f == "aiff") return 4;
                if (f == "ape" || f == "wv") return 5;
                if (f == "ogg" || f == "opus") return 6;
                if (f == "m4a" || f == "aac") return 7;
                if (f == "mp3") return 8;
                return 9;
            };
            int pA = getFormatPriority(a.format);
            int pB = getFormatPriority(b.format);
            if (pA != pB) {
                less = pA < pB;
            } else {
                less = compCaseInsensitive(a.title, b.title);
            }
        } else if (m_sortField == "favorite") {
            if (a.isFavorite != b.isFavorite) {
                less = a.isFavorite && !b.isFavorite;
            } else {
                less = compCaseInsensitive(a.title, b.title);
            }
        } else if (m_sortField == "play_count") {
            if (a.playCount != b.playCount) {
                less = a.playCount < b.playCount;
            } else {
                less = compCaseInsensitive(a.title, b.title);
            }
        } else if (m_sortField == "track_number") {
            uint64_t ordA = static_cast<uint64_t>(a.discNumber) * 10000 + a.trackNumber;
            uint64_t ordB = static_cast<uint64_t>(b.discNumber) * 10000 + b.trackNumber;
            if (ordA != ordB) {
                less = ordA < ordB;
            } else {
                less = compCaseInsensitive(a.title, b.title);
            }
        } else {
            less = compCaseInsensitive(a.title, b.title);
        }

        return m_sortAscending ? less : !less;
    });
}

void LibraryModel::refresh() {
    beginResetModel();
    m_tracks = db::DatabaseManager::instance().getAllTracks("title ASC");
    applyCurrentSort();
    endResetModel();
    emit countChanged();
}

void LibraryModel::search(const QString& query) {
    m_lastSearchQuery = query;
    beginResetModel();
    if (query.trimmed().isEmpty()) {
        m_tracks = db::DatabaseManager::instance().getAllTracks("title ASC");
    } else {
        m_tracks = db::DatabaseManager::instance().searchTracks(query.toStdString());
    }
    applyCurrentSort();
    endResetModel();
    emit countChanged();
}

void LibraryModel::filterFavorites() {
    beginResetModel();
    m_tracks = db::DatabaseManager::instance().getFavoriteTracks();
    applyCurrentSort();
    endResetModel();
    emit countChanged();
}

void LibraryModel::filterRecentlyPlayed() {
    beginResetModel();
    m_tracks = db::DatabaseManager::instance().getRecentlyPlayedTracks(50);
    if (m_sortField != "date_added" && m_sortField != "title") {
        applyCurrentSort();
    }
    endResetModel();
    emit countChanged();
}

void LibraryModel::filterRecentlyAdded() {
    beginResetModel();
    m_tracks = db::DatabaseManager::instance().getRecentlyAddedTracks(50);
    if (m_sortField != "date_added" && m_sortField != "title") {
        applyCurrentSort();
    }
    endResetModel();
    emit countChanged();
}

void LibraryModel::sortBy(const QString& field) {
    if (m_sortField == field) {
        m_sortAscending = !m_sortAscending;
    } else {
        m_sortField = field;
        if (field == "date_added" || field == "play_count" || field == "favorite") {
            m_sortAscending = false;
        } else {
            m_sortAscending = true;
        }
    }
    beginResetModel();
    applyCurrentSort();
    endResetModel();
    emit sortChanged();
}

void LibraryModel::setSort(const QString& field, bool ascending) {
    m_sortField = field;
    m_sortAscending = ascending;
    beginResetModel();
    applyCurrentSort();
    endResetModel();
    emit sortChanged();
}

void LibraryModel::shuffleList() {
    beginResetModel();
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_tracks.begin(), m_tracks.end(), g);
    m_sortField = "custom";
    endResetModel();
    emit sortChanged();
}

void LibraryModel::toggleFavorite(int row) {
    if (row < 0 || row >= static_cast<int>(m_tracks.size())) return;

    auto& track = m_tracks[row];
    track.isFavorite = !track.isFavorite;
    db::DatabaseManager::instance().setTrackFavorite(track.id, track.isFavorite);

    QModelIndex idx = index(row);
    emit dataChanged(idx, idx, {FavoriteRole});
}

QVariantMap LibraryModel::getTrackAt(int row) const {
    QVariantMap map;
    if (row < 0 || row >= static_cast<int>(m_tracks.size())) return map;

    const auto& t = m_tracks[row];
    map["id"] = static_cast<qint64>(t.id);
    map["title"] = QString::fromStdString(t.title);
    map["artist"] = QString::fromStdString(t.artist);
    map["album"] = QString::fromStdString(t.album);
    map["filePath"] = QString::fromStdString(t.filePath);
    map["duration"] = t.durationSeconds;
    map["format"] = QString::fromStdString(t.format);
    map["artworkPath"] = QString::fromStdString(t.artworkPath);
    return map;
}

std::vector<audio::QueueItem> LibraryModel::toQueueItems() const {
    std::vector<audio::QueueItem> items;
    items.reserve(m_tracks.size());
    for (const auto& t : m_tracks) {
        audio::QueueItem it;
        it.filePath = t.filePath;
        it.title = t.title;
        it.artist = t.artist;
        it.album = t.album;
        it.durationSeconds = t.durationSeconds;
        items.push_back(std::move(it));
    }
    return items;
}

} // namespace soniq::services
