#include "LibraryModel.hpp"

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

void LibraryModel::refresh() {
    beginResetModel();
    m_tracks = db::DatabaseManager::instance().getAllTracks("title ASC");
    endResetModel();
    emit countChanged();
}

void LibraryModel::search(const QString& query) {
    beginResetModel();
    if (query.trimmed().isEmpty()) {
        m_tracks = db::DatabaseManager::instance().getAllTracks("title ASC");
    } else {
        m_tracks = db::DatabaseManager::instance().searchTracks(query.toStdString());
    }
    endResetModel();
    emit countChanged();
}

void LibraryModel::filterFavorites() {
    beginResetModel();
    m_tracks = db::DatabaseManager::instance().getFavoriteTracks();
    endResetModel();
    emit countChanged();
}

void LibraryModel::filterRecentlyPlayed() {
    beginResetModel();
    m_tracks = db::DatabaseManager::instance().getRecentlyPlayedTracks(50);
    endResetModel();
    emit countChanged();
}

void LibraryModel::filterRecentlyAdded() {
    beginResetModel();
    m_tracks = db::DatabaseManager::instance().getRecentlyAddedTracks(50);
    endResetModel();
    emit countChanged();
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

} // namespace soniq::services
