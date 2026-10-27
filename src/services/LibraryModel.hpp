#pragma once

#include "database/DatabaseManager.hpp"
#include <QAbstractListModel>
#include <vector>

namespace soniq::services {

class LibraryModel : public QAbstractListModel {
    Q_OBJECT

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
        PlayCountRole
    };

    explicit LibraryModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

public slots:
    void refresh();
    void search(const QString& query);
    void filterFavorites();
    void filterRecentlyPlayed();
    void filterRecentlyAdded();
    void toggleFavorite(int row);
    QVariantMap getTrackAt(int row) const;

signals:
    void countChanged();

private:
    std::vector<db::TrackRecord> m_tracks;
};

} // namespace soniq::services
