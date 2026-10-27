#include "DatabaseManager.hpp"
#include "core/Logger.hpp"
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

namespace soniq::db {

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager s_instance;
    return s_instance;
}

DatabaseManager::DatabaseManager()
    : m_connectionName("soniq_primary_db") {}

DatabaseManager::~DatabaseManager() {
    close();
}

bool DatabaseManager::initialize(const QString& dbPath) {
    if (m_db.isOpen()) return true;

    QString finalPath = dbPath;
    if (finalPath.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        finalPath = dataDir + "/soniq_library.db";
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(finalPath);

    if (!m_db.open()) {
        SONIQ_LOG_ERROR("Failed to open SQLite database: {}", m_db.lastError().text().toStdString());
        return false;
    }

    SONIQ_LOG_INFO("SQLite database opened successfully at: {}", finalPath.toStdString());
    return runMigrations();
}

void DatabaseManager::close() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::isOpen() const {
    return m_db.isOpen();
}

bool DatabaseManager::runMigrations() {
    QSqlQuery query(m_db);

    // Schema version tracking table
    query.exec("CREATE TABLE IF NOT EXISTS schema_version (version INTEGER PRIMARY KEY);");

    int currentVersion = 0;
    if (query.exec("SELECT version FROM schema_version LIMIT 1;") && query.next()) {
        currentVersion = query.value(0).toInt();
    } else {
        query.exec("INSERT INTO schema_version (version) VALUES (0);");
    }

    if (currentVersion < 1) {
        SONIQ_LOG_INFO("Running database schema migration to v1...");
        m_db.transaction();

        // 1. Tracks table
        bool ok = query.exec(
            "CREATE TABLE IF NOT EXISTS tracks ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  file_path TEXT UNIQUE NOT NULL,"
            "  title TEXT NOT NULL,"
            "  artist TEXT,"
            "  album TEXT,"
            "  album_artist TEXT,"
            "  genre TEXT,"
            "  year INTEGER,"
            "  track_number INTEGER,"
            "  disc_number INTEGER DEFAULT 1,"
            "  duration REAL NOT NULL,"
            "  sample_rate INTEGER,"
            "  bit_depth INTEGER,"
            "  channels INTEGER,"
            "  bitrate INTEGER,"
            "  format TEXT,"
            "  file_size INTEGER,"
            "  date_added INTEGER,"
            "  play_count INTEGER DEFAULT 0,"
            "  last_played INTEGER DEFAULT 0,"
            "  favorite INTEGER DEFAULT 0,"
            "  rating INTEGER DEFAULT 0,"
            "  artwork_path TEXT"
            ");"
        );

        // 2. Playlists table
        ok &= query.exec(
            "CREATE TABLE IF NOT EXISTS playlists ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  name TEXT NOT NULL,"
            "  created_at INTEGER,"
            "  updated_at INTEGER,"
            "  is_smart INTEGER DEFAULT 0,"
            "  rules_json TEXT"
            ");"
        );

        // 3. Playlist Tracks mapping table
        ok &= query.exec(
            "CREATE TABLE IF NOT EXISTS playlist_tracks ("
            "  playlist_id INTEGER,"
            "  track_id INTEGER,"
            "  position INTEGER,"
            "  PRIMARY KEY (playlist_id, track_id),"
            "  FOREIGN KEY (playlist_id) REFERENCES playlists(id) ON DELETE CASCADE,"
            "  FOREIGN KEY (track_id) REFERENCES tracks(id) ON DELETE CASCADE"
            ");"
        );

        // 4. Listening history table
        ok &= query.exec(
            "CREATE TABLE IF NOT EXISTS listening_history ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  track_id INTEGER,"
            "  played_at INTEGER,"
            "  completed_pct REAL,"
            "  FOREIGN KEY (track_id) REFERENCES tracks(id) ON DELETE CASCADE"
            ");"
        );

        // 5. User Preferences table
        ok &= query.exec(
            "CREATE TABLE IF NOT EXISTS preferences ("
            "  key TEXT PRIMARY KEY,"
            "  value TEXT"
            ");"
        );

        // 6. Performance indexes
        ok &= query.exec("CREATE INDEX IF NOT EXISTS idx_tracks_artist ON tracks(artist);");
        ok &= query.exec("CREATE INDEX IF NOT EXISTS idx_tracks_album ON tracks(album);");
        ok &= query.exec("CREATE INDEX IF NOT EXISTS idx_tracks_title ON tracks(title);");
        ok &= query.exec("CREATE INDEX IF NOT EXISTS idx_tracks_genre ON tracks(genre);");
        ok &= query.exec("CREATE INDEX IF NOT EXISTS idx_tracks_favorite ON tracks(favorite);");
        ok &= query.exec("CREATE INDEX IF NOT EXISTS idx_tracks_play_count ON tracks(play_count);");

        if (ok) {
            query.exec("UPDATE schema_version SET version = 1;");
            m_db.commit();
            SONIQ_LOG_INFO("Database migration v1 completed successfully.");
            currentVersion = 1;
        } else {
            m_db.rollback();
            SONIQ_LOG_ERROR("Migration v1 failed: {}", query.lastError().text().toStdString());
            return false;
        }
    }

    if (currentVersion < 2) {
        SONIQ_LOG_INFO("Running database schema migration to v2 (artwork_path)...");
        m_db.transaction();
        QSqlQuery check(m_db);
        check.exec("PRAGMA table_info(tracks);");
        bool hasArtCol = false;
        while (check.next()) {
            if (check.value(1).toString() == "artwork_path") {
                hasArtCol = true;
                break;
            }
        }
        bool ok = true;
        if (!hasArtCol) {
            ok = query.exec("ALTER TABLE tracks ADD COLUMN artwork_path TEXT;");
        }
        if (ok) {
            query.exec("UPDATE schema_version SET version = 2;");
            m_db.commit();
            SONIQ_LOG_INFO("Database migration v2 completed successfully.");
        } else {
            m_db.rollback();
            SONIQ_LOG_ERROR("Migration v2 failed: {}", query.lastError().text().toStdString());
            return false;
        }
    }

    return true;
}

int64_t DatabaseManager::insertOrUpdateTrack(const TrackRecord& track) {
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO tracks ("
        "  file_path, title, artist, album, album_artist, genre, year, track_number, disc_number,"
        "  duration, sample_rate, bit_depth, channels, bitrate, format, file_size, date_added,"
        "  play_count, last_played, favorite, rating, artwork_path"
        ") VALUES ("
        "  :path, :title, :artist, :album, :album_artist, :genre, :year, :track_num, :disc_num,"
        "  :dur, :sr, :bd, :ch, :br, :fmt, :sz, :added, :pc, :lp, :fav, :rate, :art"
        ") ON CONFLICT(file_path) DO UPDATE SET "
        "  title = excluded.title, artist = excluded.artist, album = excluded.album,"
        "  duration = excluded.duration, bitrate = excluded.bitrate, file_size = excluded.file_size,"
        "  artwork_path = excluded.artwork_path;"
    );

    query.bindValue(":path", QString::fromStdString(track.filePath));
    query.bindValue(":title", QString::fromStdString(track.title));
    query.bindValue(":artist", QString::fromStdString(track.artist));
    query.bindValue(":album", QString::fromStdString(track.album));
    query.bindValue(":album_artist", QString::fromStdString(track.albumArtist));
    query.bindValue(":genre", QString::fromStdString(track.genre));
    query.bindValue(":year", track.year);
    query.bindValue(":track_num", track.trackNumber);
    query.bindValue(":disc_num", track.discNumber);
    query.bindValue(":dur", track.durationSeconds);
    query.bindValue(":sr", track.sampleRate);
    query.bindValue(":bd", track.bitDepth);
    query.bindValue(":ch", track.channels);
    query.bindValue(":br", track.bitrate);
    query.bindValue(":fmt", QString::fromStdString(track.format));
    query.bindValue(":sz", static_cast<qulonglong>(track.fileSize));
    query.bindValue(":added", track.dateAdded > 0 ? track.dateAdded : QDateTime::currentSecsSinceEpoch());
    query.bindValue(":pc", track.playCount);
    query.bindValue(":lp", track.lastPlayed);
    query.bindValue(":fav", track.isFavorite ? 1 : 0);
    query.bindValue(":rate", track.rating);
    query.bindValue(":art", QString::fromStdString(track.artworkPath));

    if (query.exec()) {
        return query.lastInsertId().toLongLong();
    } else {
        SONIQ_LOG_ERROR("Failed to insert/update track: {}", query.lastError().text().toStdString());
        return -1;
    }
}

bool DatabaseManager::removeTrack(int64_t trackId) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM tracks WHERE id = :id;");
    query.bindValue(":id", static_cast<qint64>(trackId));
    return query.exec();
}

bool DatabaseManager::removeTrackByPath(const std::string& filePath) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM tracks WHERE file_path = :path;");
    query.bindValue(":path", QString::fromStdString(filePath));
    return query.exec();
}

static TrackRecord readTrackRecord(const QSqlQuery& q) {
    TrackRecord r;
    r.id = q.value("id").toLongLong();
    r.filePath = q.value("file_path").toString().toStdString();
    r.title = q.value("title").toString().toStdString();
    r.artist = q.value("artist").toString().toStdString();
    r.album = q.value("album").toString().toStdString();
    r.albumArtist = q.value("album_artist").toString().toStdString();
    r.genre = q.value("genre").toString().toStdString();
    r.year = q.value("year").toUInt();
    r.trackNumber = q.value("track_number").toUInt();
    r.discNumber = q.value("disc_number").toUInt();
    r.durationSeconds = q.value("duration").toDouble();
    r.sampleRate = q.value("sample_rate").toUInt();
    r.bitDepth = static_cast<uint16_t>(q.value("bit_depth").toUInt());
    r.channels = static_cast<uint16_t>(q.value("channels").toUInt());
    r.bitrate = q.value("bitrate").toUInt();
    r.format = q.value("format").toString().toStdString();
    r.fileSize = q.value("file_size").toULongLong();
    r.dateAdded = q.value("date_added").toLongLong();
    r.playCount = q.value("play_count").toUInt();
    r.lastPlayed = q.value("last_played").toLongLong();
    r.isFavorite = q.value("favorite").toInt() != 0;
    r.rating = q.value("rating").toInt();
    r.artworkPath = q.value("artwork_path").toString().toStdString();
    return r;
}

std::vector<TrackRecord> DatabaseManager::getAllTracks(const std::string& sortBy) {
    std::vector<TrackRecord> results;
    QSqlQuery query(m_db);
    QString sql = "SELECT * FROM tracks ORDER BY " + QString::fromStdString(sortBy) + ";";
    if (query.exec(sql)) {
        while (query.next()) {
            results.push_back(readTrackRecord(query));
        }
    }
    return results;
}

std::vector<TrackRecord> DatabaseManager::searchTracks(const std::string& queryString) {
    std::vector<TrackRecord> results;
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT * FROM tracks WHERE "
        "title LIKE :q OR artist LIKE :q OR album LIKE :q OR genre LIKE :q "
        "ORDER BY title ASC LIMIT 100;"
    );
    QString wildcard = "%" + QString::fromStdString(queryString) + "%";
    query.bindValue(":q", wildcard);

    if (query.exec()) {
        while (query.next()) {
            results.push_back(readTrackRecord(query));
        }
    }
    return results;
}

std::vector<TrackRecord> DatabaseManager::getFavoriteTracks() {
    std::vector<TrackRecord> results;
    QSqlQuery query(m_db);
    if (query.exec("SELECT * FROM tracks WHERE favorite = 1 ORDER BY title ASC;")) {
        while (query.next()) {
            results.push_back(readTrackRecord(query));
        }
    }
    return results;
}

std::vector<TrackRecord> DatabaseManager::getRecentlyPlayedTracks(size_t limit) {
    std::vector<TrackRecord> results;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM tracks WHERE last_played > 0 ORDER BY last_played DESC LIMIT :lim;");
    query.bindValue(":lim", static_cast<qulonglong>(limit));
    if (query.exec()) {
        while (query.next()) {
            results.push_back(readTrackRecord(query));
        }
    }
    return results;
}

std::vector<TrackRecord> DatabaseManager::getMostPlayedTracks(size_t limit) {
    std::vector<TrackRecord> results;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM tracks WHERE play_count > 0 ORDER BY play_count DESC LIMIT :lim;");
    query.bindValue(":lim", static_cast<qulonglong>(limit));
    if (query.exec()) {
        while (query.next()) {
            results.push_back(readTrackRecord(query));
        }
    }
    return results;
}

std::vector<TrackRecord> DatabaseManager::getRecentlyAddedTracks(size_t limit) {
    std::vector<TrackRecord> results;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM tracks ORDER BY date_added DESC LIMIT :lim;");
    query.bindValue(":lim", static_cast<qulonglong>(limit));
    if (query.exec()) {
        while (query.next()) {
            results.push_back(readTrackRecord(query));
        }
    }
    return results;
}

bool DatabaseManager::setTrackFavorite(int64_t trackId, bool favorite) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE tracks SET favorite = :fav WHERE id = :id;");
    query.bindValue(":fav", favorite ? 1 : 0);
    query.bindValue(":id", static_cast<qint64>(trackId));
    return query.exec();
}

bool DatabaseManager::incrementPlayCount(int64_t trackId) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE tracks SET play_count = play_count + 1, last_played = :now WHERE id = :id;");
    query.bindValue(":now", QDateTime::currentSecsSinceEpoch());
    query.bindValue(":id", static_cast<qint64>(trackId));
    return query.exec();
}

int64_t DatabaseManager::createPlaylist(const std::string& name, bool isSmart, const std::string& rules) {
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO playlists (name, created_at, updated_at, is_smart, rules_json) "
        "VALUES (:name, :created, :updated, :smart, :rules);"
    );
    int64_t now = QDateTime::currentSecsSinceEpoch();
    query.bindValue(":name", QString::fromStdString(name));
    query.bindValue(":created", now);
    query.bindValue(":updated", now);
    query.bindValue(":smart", isSmart ? 1 : 0);
    query.bindValue(":rules", QString::fromStdString(rules));

    if (query.exec()) {
        return query.lastInsertId().toLongLong();
    }
    return -1;
}

bool DatabaseManager::deletePlaylist(int64_t playlistId) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM playlists WHERE id = :id;");
    query.bindValue(":id", static_cast<qint64>(playlistId));
    return query.exec();
}

bool DatabaseManager::renamePlaylist(int64_t playlistId, const std::string& newName) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE playlists SET name = :name, updated_at = :up WHERE id = :id;");
    query.bindValue(":name", QString::fromStdString(newName));
    query.bindValue(":up", QDateTime::currentSecsSinceEpoch());
    query.bindValue(":id", static_cast<qint64>(playlistId));
    return query.exec();
}

std::vector<PlaylistRecord> DatabaseManager::getAllPlaylists() {
    std::vector<PlaylistRecord> results;
    QSqlQuery query(m_db);
    if (query.exec("SELECT p.*, COUNT(pt.track_id) as tcount FROM playlists p "
                   "LEFT JOIN playlist_tracks pt ON p.id = pt.playlist_id "
                   "GROUP BY p.id ORDER BY p.name ASC;")) {
        while (query.next()) {
            PlaylistRecord pl;
            pl.id = query.value("id").toLongLong();
            pl.name = query.value("name").toString().toStdString();
            pl.createdAt = query.value("created_at").toLongLong();
            pl.updatedAt = query.value("updated_at").toLongLong();
            pl.isSmart = query.value("is_smart").toInt() != 0;
            pl.rulesJson = query.value("rules_json").toString().toStdString();
            pl.trackCount = query.value("tcount").toULongLong();
            results.push_back(std::move(pl));
        }
    }
    return results;
}

bool DatabaseManager::addTrackToPlaylist(int64_t playlistId, int64_t trackId) {
    QSqlQuery query(m_db);
    query.prepare("INSERT OR IGNORE INTO playlist_tracks (playlist_id, track_id, position) "
                  "VALUES (:pl, :tr, (SELECT COALESCE(MAX(position), 0) + 1 FROM playlist_tracks WHERE playlist_id = :pl));");
    query.bindValue(":pl", static_cast<qint64>(playlistId));
    query.bindValue(":tr", static_cast<qint64>(trackId));
    return query.exec();
}

bool DatabaseManager::removeTrackFromPlaylist(int64_t playlistId, int64_t trackId) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM playlist_tracks WHERE playlist_id = :pl AND track_id = :tr;");
    query.bindValue(":pl", static_cast<qint64>(playlistId));
    query.bindValue(":tr", static_cast<qint64>(trackId));
    return query.exec();
}

std::vector<TrackRecord> DatabaseManager::getPlaylistTracks(int64_t playlistId) {
    std::vector<TrackRecord> results;
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT t.* FROM tracks t "
        "INNER JOIN playlist_tracks pt ON t.id = pt.track_id "
        "WHERE pt.playlist_id = :pl "
        "ORDER BY pt.position ASC;"
    );
    query.bindValue(":pl", static_cast<qint64>(playlistId));
    if (query.exec()) {
        while (query.next()) {
            results.push_back(readTrackRecord(query));
        }
    }
    return results;
}

void DatabaseManager::setPreference(const std::string& key, const std::string& value) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO preferences (key, value) VALUES (:k, :v) "
                  "ON CONFLICT(key) DO UPDATE SET value = excluded.value;");
    query.bindValue(":k", QString::fromStdString(key));
    query.bindValue(":v", QString::fromStdString(value));
    query.exec();
}

std::string DatabaseManager::getPreference(const std::string& key, const std::string& defaultValue) {
    QSqlQuery query(m_db);
    query.prepare("SELECT value FROM preferences WHERE key = :k LIMIT 1;");
    query.bindValue(":k", QString::fromStdString(key));
    if (query.exec() && query.next()) {
        return query.value(0).toString().toStdString();
    }
    return defaultValue;
}

} // namespace soniq::db
