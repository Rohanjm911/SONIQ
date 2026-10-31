#include "DatabaseManager.hpp"
#include "core/Logger.hpp"
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QThread>

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
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_db.isOpen()) return true;

    m_mainThreadId = QThread::currentThreadId();

    QString finalPath = dbPath;
    if (finalPath.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        finalPath = dataDir + "/soniq_library.db";
    }
    m_dbPath = finalPath;

    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(m_dbPath);

    if (!m_db.open()) {
        SONIQ_LOG_ERROR("Failed to open SQLite database: {}", m_db.lastError().text().toStdString());
        return false;
    }

    // Enable WAL mode for high performance & multi-threaded concurrency
    {
        QSqlQuery walQuery(m_db);
        walQuery.exec("PRAGMA journal_mode = WAL;");
        walQuery.exec("PRAGMA synchronous = NORMAL;");
    }

    SONIQ_LOG_INFO("SQLite database opened successfully at: {}", finalPath.toStdString());
    return runMigrations();
}

void DatabaseManager::close() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::isOpen() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_db.isOpen();
}

QSqlDatabase DatabaseManager::getDatabase() const {
    if (QThread::currentThreadId() == m_mainThreadId && m_db.isOpen()) {
        return m_db;
    }

    const QString threadConnName = QString("%1_thread_%2")
                                   .arg(m_connectionName)
                                   .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    if (QSqlDatabase::contains(threadConnName)) {
        QSqlDatabase db = QSqlDatabase::database(threadConnName);
        if (db.isOpen()) {
            return db;
        }
    }

    QSqlDatabase threadDb = QSqlDatabase::addDatabase("QSQLITE", threadConnName);
    threadDb.setDatabaseName(m_dbPath);
    if (!threadDb.open()) {
        SONIQ_LOG_ERROR("Failed to open thread database connection {}: {}",
                        threadConnName.toStdString(), threadDb.lastError().text().toStdString());
        return m_db;
    }

    return threadDb;
}

bool DatabaseManager::runMigrations() {
    QSqlDatabase db = getDatabase();
    QSqlQuery query(db);

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
        db.transaction();

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
            db.commit();
            SONIQ_LOG_INFO("Database migration v1 completed successfully.");
            currentVersion = 1;
        } else {
            db.rollback();
            SONIQ_LOG_ERROR("Migration v1 failed: {}", query.lastError().text().toStdString());
            return false;
        }
    }

    if (currentVersion < 2) {
        SONIQ_LOG_INFO("Running database schema migration to v2 (artwork_path)...");
        db.transaction();
        QSqlQuery check(db);
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
            db.commit();
            SONIQ_LOG_INFO("Database migration v2 completed successfully.");
        } else {
            db.rollback();
            SONIQ_LOG_ERROR("Migration v2 failed: {}", query.lastError().text().toStdString());
            return false;
        }
    }

    return true;
}

int64_t DatabaseManager::insertOrUpdateTrack(const TrackRecord& track) {
    std::lock_guard<std::mutex> lock(m_mutex);
    QSqlDatabase db = getDatabase();
    QSqlQuery query(db);
    if (!query.prepare(
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
    )) {
        SONIQ_LOG_ERROR("Failed to prepare insert/update track statement: {}", query.lastError().text().toStdString());
        return -1;
    }

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
        auto insertId = query.lastInsertId().toLongLong();
        return insertId > 0 ? insertId : 1;
    } else {
        SONIQ_LOG_ERROR("Failed to insert/update track: {}", query.lastError().text().toStdString());
        return -1;
    }
}

bool DatabaseManager::removeTrack(int64_t trackId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    QSqlQuery query(getDatabase());
    query.prepare("DELETE FROM tracks WHERE id = :id;");
    query.bindValue(":id", static_cast<qint64>(trackId));
    return query.exec();
}

bool DatabaseManager::removeTrackByPath(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    QSqlQuery query(getDatabase());
    query.prepare("DELETE FROM tracks WHERE file_path = :path;");
    query.bindValue(":path", QString::fromStdString(filePath));
    return query.exec();
}

bool DatabaseManager::clearAllTracks() {
    std::lock_guard<std::mutex> lock(m_mutex);
    QSqlQuery query(getDatabase());
    query.exec("DELETE FROM playlist_tracks;");
    return query.exec("DELETE FROM tracks;");
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
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<TrackRecord> results;
    QSqlQuery query(getDatabase());
    QString sql = "SELECT * FROM tracks ORDER BY " + QString::fromStdString(sortBy) + ";";
    if (query.exec(sql)) {
        while (query.next()) {
            results.push_back(readTrackRecord(query));
        }
    }
    return results;
}

std::vector<TrackRecord> DatabaseManager::searchTracks(const std::string& queryString) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<TrackRecord> results;
    QSqlQuery query(getDatabase());
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
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<TrackRecord> results;
    QSqlQuery query(getDatabase());
    if (query.exec("SELECT * FROM tracks WHERE favorite = 1 ORDER BY title ASC;")) {
        while (query.next()) {
            results.push_back(readTrackRecord(query));
        }
    }
    return results;
}

std::vector<TrackRecord> DatabaseManager::getRecentlyPlayedTracks(size_t limit) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<TrackRecord> results;
    QSqlQuery query(getDatabase());
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
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<TrackRecord> results;
    QSqlQuery query(getDatabase());
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
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<TrackRecord> results;
    QSqlQuery query(getDatabase());
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
    std::lock_guard<std::mutex> lock(m_mutex);
    QSqlQuery query(getDatabase());
    query.prepare("UPDATE tracks SET favorite = :fav WHERE id = :id;");
    query.bindValue(":fav", favorite ? 1 : 0);
    query.bindValue(":id", static_cast<qint64>(trackId));
    return query.exec();
}

bool DatabaseManager::incrementPlayCount(int64_t trackId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    QSqlQuery query(getDatabase());
    query.prepare("UPDATE tracks SET play_count = play_count + 1, last_played = :now WHERE id = :id;");
    query.bindValue(":now", QDateTime::currentSecsSinceEpoch());
    query.bindValue(":id", static_cast<qint64>(trackId));
    return query.exec();
}

bool DatabaseManager::updateTrackMetadata(int64_t trackId, const std::string& title, const std::string& artist,
                                         const std::string& album, const std::string& genre, uint32_t year) {
    std::lock_guard<std::mutex> lock(m_mutex);
    QSqlQuery query(getDatabase());
    query.prepare(
        "UPDATE tracks SET title = :title, artist = :artist, album = :album, genre = :genre, year = :year "
        "WHERE id = :id;"
    );
    query.bindValue(":title", QString::fromStdString(title));
    query.bindValue(":artist", QString::fromStdString(artist));
    query.bindValue(":album", QString::fromStdString(album));
    query.bindValue(":genre", QString::fromStdString(genre));
    query.bindValue(":year", year);
    query.bindValue(":id", static_cast<qint64>(trackId));
    return query.exec();
}

int64_t DatabaseManager::createPlaylist(const std::string& name, bool isSmart, const std::string& rules) {
    std::lock_guard<std::mutex> lock(m_mutex);
    QSqlQuery query(getDatabase());
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
    std::lock_guard<std::mutex> lock(m_mutex);
    QSqlQuery query(getDatabase());
    query.prepare("DELETE FROM playlists WHERE id = :id;");
    query.bindValue(":id", static_cast<qint64>(playlistId));
    return query.exec();
}

bool DatabaseManager::renamePlaylist(int64_t playlistId, const std::string& newName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    QSqlQuery query(getDatabase());
    query.prepare("UPDATE playlists SET name = :name, updated_at = :up WHERE id = :id;");
    query.bindValue(":name", QString::fromStdString(newName));
    query.bindValue(":up", QDateTime::currentSecsSinceEpoch());
    query.bindValue(":id", static_cast<qint64>(playlistId));
    return query.exec();
}

std::vector<PlaylistRecord> DatabaseManager::getAllPlaylists() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<PlaylistRecord> results;
    QSqlQuery query(getDatabase());
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
    std::lock_guard<std::mutex> lock(m_mutex);
    QSqlQuery query(getDatabase());
    query.prepare("INSERT OR IGNORE INTO playlist_tracks (playlist_id, track_id, position) "
                  "VALUES (:pl, :tr, (SELECT COALESCE(MAX(position), 0) + 1 FROM playlist_tracks WHERE playlist_id = :pl));");
    query.bindValue(":pl", static_cast<qint64>(playlistId));
    query.bindValue(":tr", static_cast<qint64>(trackId));
    return query.exec();
}

bool DatabaseManager::removeTrackFromPlaylist(int64_t playlistId, int64_t trackId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    QSqlQuery query(getDatabase());
    query.prepare("DELETE FROM playlist_tracks WHERE playlist_id = :pl AND track_id = :tr;");
    query.bindValue(":pl", static_cast<qint64>(playlistId));
    query.bindValue(":tr", static_cast<qint64>(trackId));
    return query.exec();
}

std::vector<TrackRecord> DatabaseManager::getPlaylistTracks(int64_t playlistId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<TrackRecord> results;
    QSqlQuery query(getDatabase());
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
    std::lock_guard<std::mutex> lock(m_mutex);
    QSqlQuery query(getDatabase());
    query.prepare("INSERT INTO preferences (key, value) VALUES (:k, :v) "
                  "ON CONFLICT(key) DO UPDATE SET value = excluded.value;");
    query.bindValue(":k", QString::fromStdString(key));
    query.bindValue(":v", QString::fromStdString(value));
    query.exec();
}

std::string DatabaseManager::getPreference(const std::string& key, const std::string& defaultValue) {
    std::lock_guard<std::mutex> lock(m_mutex);
    QSqlQuery query(getDatabase());
    query.prepare("SELECT value FROM preferences WHERE key = :k LIMIT 1;");
    query.bindValue(":k", QString::fromStdString(key));
    if (query.exec() && query.next()) {
        return query.value(0).toString().toStdString();
    }
    return defaultValue;
}

std::vector<DatabaseManager::AlbumSummary> DatabaseManager::getDistinctAlbums() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<AlbumSummary> results;
    QSqlQuery query(getDatabase());
    QString sql = "SELECT album, artist, COUNT(*) as cnt, MAX(artwork_path) as art, MAX(year) as yr "
                  "FROM tracks WHERE album IS NOT NULL AND album != '' "
                  "GROUP BY album ORDER BY album COLLATE NOCASE ASC;";
    if (query.exec(sql)) {
        while (query.next()) {
            AlbumSummary a;
            a.album = query.value(0).toString().toStdString();
            a.artist = query.value(1).toString().toStdString();
            a.trackCount = query.value(2).toUInt();
            a.artworkPath = query.value(3).toString().toStdString();
            a.year = query.value(4).toUInt();
            results.push_back(std::move(a));
        }
    }
    return results;
}

std::vector<DatabaseManager::ArtistSummary> DatabaseManager::getDistinctArtists() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<ArtistSummary> results;
    QSqlQuery query(getDatabase());
    QString sql = "SELECT artist, COUNT(*) as cnt, COUNT(DISTINCT album) as alb_cnt "
                  "FROM tracks WHERE artist IS NOT NULL AND artist != '' "
                  "GROUP BY artist ORDER BY artist COLLATE NOCASE ASC;";
    if (query.exec(sql)) {
        while (query.next()) {
            ArtistSummary a;
            a.artist = query.value(0).toString().toStdString();
            a.trackCount = query.value(1).toUInt();
            a.albumCount = query.value(2).toUInt();
            results.push_back(std::move(a));
        }
    }
    return results;
}

std::vector<DatabaseManager::GenreSummary> DatabaseManager::getDistinctGenres() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<GenreSummary> results;
    QSqlQuery query(getDatabase());
    QString sql = "SELECT genre, COUNT(*) as cnt "
                  "FROM tracks WHERE genre IS NOT NULL AND genre != '' "
                  "GROUP BY genre ORDER BY genre COLLATE NOCASE ASC;";
    if (query.exec(sql)) {
        while (query.next()) {
            GenreSummary g;
            g.genre = query.value(0).toString().toStdString();
            g.trackCount = query.value(1).toUInt();
            results.push_back(std::move(g));
        }
    }
    return results;
}

std::vector<TrackRecord> DatabaseManager::getTracksByAlbum(const std::string& album) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<TrackRecord> results;
    QSqlQuery query(getDatabase());
    query.prepare("SELECT * FROM tracks WHERE album = :alb ORDER BY disc_number ASC, track_number ASC, title ASC;");
    query.bindValue(":alb", QString::fromStdString(album));
    if (query.exec()) {
        while (query.next()) {
            results.push_back(readTrackRecord(query));
        }
    }
    return results;
}

std::vector<TrackRecord> DatabaseManager::getTracksByArtist(const std::string& artist) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<TrackRecord> results;
    QSqlQuery query(getDatabase());
    query.prepare("SELECT * FROM tracks WHERE artist = :art ORDER BY album ASC, disc_number ASC, track_number ASC, title ASC;");
    query.bindValue(":art", QString::fromStdString(artist));
    if (query.exec()) {
        while (query.next()) {
            results.push_back(readTrackRecord(query));
        }
    }
    return results;
}

std::vector<TrackRecord> DatabaseManager::getTracksByGenre(const std::string& genre) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<TrackRecord> results;
    QSqlQuery query(getDatabase());
    query.prepare("SELECT * FROM tracks WHERE genre = :gen ORDER BY artist ASC, album ASC, track_number ASC;");
    query.bindValue(":gen", QString::fromStdString(genre));
    if (query.exec()) {
        while (query.next()) {
            results.push_back(readTrackRecord(query));
        }
    }
    return results;
}

} // namespace soniq::db
