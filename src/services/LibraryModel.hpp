#pragma once

#include "database/DatabaseManager.hpp"
#include <QAbstractListModel>
#include <vector>

namespace soniq::services {

class LibraryModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QString sortField READ sortField NOTIFY sortChanged)
    Q_PROPERTY(bool sortAscending READ sortAscending NOTIFY sortChanged)
    Q_PROPERTY(QString sortLabel READ sortLabel NOTIFY sortChanged)

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

public slots:
    void refresh();
    void search(const QString& query);
    void filterFavorites();
    void filterRecentlyPlayed();
    void filterRecentlyAdded();
    void toggleFavorite(int row);
    QVariantMap getTrackAt(int row) const;

    void sortBy(const QString& field);
    void setSort(const QString& field, bool ascending);
    void shuffleList();

signals:
    void countChanged();
    void sortChanged();

private:
    void applyCurrentSort();

    std::vector<db::TrackRecord> m_tracks;
    QString m_sortField{"title"};
    bool m_sortAscending{true};
    QString m_lastSearchQuery;
};

} // namespace soniq::services
