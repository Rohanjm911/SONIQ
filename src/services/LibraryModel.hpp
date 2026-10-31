#pragma once

#include "database/DatabaseManager.hpp"
#include "audio/PlaybackQueue.hpp"
#include <QAbstractListModel>
#include <vector>

namespace soniq::services {

class LibraryModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QString sortField READ sortField NOTIFY sortChanged)
    Q_PROPERTY(bool sortAscending READ sortAscending NOTIFY sortChanged)
    Q_PROPERTY(QString sortLabel READ sortLabel NOTIFY sortChanged)
    Q_PROPERTY(QVariantList playlists READ getPlaylists NOTIFY playlistsChanged)
    Q_PROPERTY(qint64 activePlaylistId READ activePlaylistId NOTIFY activePlaylistChanged)
    Q_PROPERTY(QString activePlaylistName READ activePlaylistName NOTIFY activePlaylistChanged)
    Q_PROPERTY(QString activeFilterType READ activeFilterType NOTIFY activeFilterChanged)
    Q_PROPERTY(QString activeFilterValue READ activeFilterValue NOTIFY activeFilterChanged)

public:
    enum TrackRoles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        ArtistRole,
        AlbumRole,
        GenreRole,
        YearRole,
        DurationRole,
        DurationFormattedRole,
        FilePathRole,
        FormatRole,
        FavoriteRole,
        PlayCountRole,
        ArtworkPathRole
    };

    explicit LibraryModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] QString sortField() const { return m_sortField; }
    [[nodiscard]] bool sortAscending() const { return m_sortAscending; }
    [[nodiscard]] QString sortLabel() const;
    [[nodiscard]] std::vector<audio::QueueItem> toQueueItems() const;

    [[nodiscard]] QVariantList getPlaylists() const;
    [[nodiscard]] qint64 activePlaylistId() const { return m_activePlaylistId; }
    [[nodiscard]] QString activePlaylistName() const { return m_activePlaylistName; }
    [[nodiscard]] QString activeFilterType() const { return m_activeFilterType; }
    [[nodiscard]] QString activeFilterValue() const { return m_activeFilterValue; }

    Q_INVOKABLE QVariantList getAlbums() const;
    Q_INVOKABLE QVariantList getArtists() const;
    Q_INVOKABLE QVariantList getGenres() const;

public slots:
    void refresh();
    void search(const QString& query);
    void filterFavorites();
    void filterRecentlyPlayed();
    void filterRecentlyAdded();
    void filterMostPlayed();
    void filterByAlbum(const QString& album);
    void filterByArtist(const QString& artist);
    void filterByGenre(const QString& genre);
    void clearFilter();
    void toggleFavorite(int row);
    QVariantMap getTrackAt(int row) const;

    void sortBy(const QString& field);
    void setSort(const QString& field, bool ascending);
    void shuffleList();
    void clearLibrary();

    // Playlist Management (Feature 2)
    qint64 createPlaylist(const QString& name);
    bool deletePlaylist(qint64 playlistId);
    bool renamePlaylist(qint64 playlistId, const QString& newName);
    bool addTrackToPlaylist(qint64 playlistId, qint64 trackId);
    bool removeTrackFromPlaylist(qint64 playlistId, qint64 trackId);
    void filterByPlaylist(qint64 playlistId, const QString& playlistName);
    void clearPlaylistFilter();

    // Built-in Audio Tag & Metadata Editor (Feature 6)
    bool updateTrackMetadata(qint64 trackId, const QString& title, const QString& artist,
                             const QString& album, const QString& genre, int year);

signals:
    void countChanged();
    void sortChanged();
    void playlistsChanged();
    void activePlaylistChanged();
    void activeFilterChanged();

private:
    void applyCurrentSort();

    std::vector<db::TrackRecord> m_tracks;
    QString m_sortField{"title"};
    bool m_sortAscending{true};
    QString m_lastSearchQuery;
    qint64 m_activePlaylistId{-1};
    QString m_activePlaylistName;
    QString m_activeFilterType;
    QString m_activeFilterValue;
};

} // namespace soniq::services

