#include "database/DatabaseManager.hpp"
#include <QCoreApplication>
#include <cassert>
#include <iostream>

void testDatabase() {
    using namespace soniq::db;

    std::cout << "\n=== Running Database & Library Storage Unit Tests ===" << std::endl;

    auto& db = DatabaseManager::instance();
    // Use an in-memory SQLite database for deterministic test execution
    bool ok = db.initialize(":memory:");
    assert(ok);
    assert(db.isOpen());

    // 1. Insert Track
    TrackRecord t1;
    t1.filePath = "/music/audiophile/song1.flac";
    t1.title = "High Resolution Symphony";
    t1.artist = "London Philharmonic";
    t1.album = "Lossless Classics";
    t1.genre = "Classical";
    t1.year = 2024;
    t1.durationSeconds = 345.5;
    t1.sampleRate = 96000;
    t1.bitDepth = 24;
    t1.channels = 2;
    t1.format = "FLAC";

    int64_t id1 = db.insertOrUpdateTrack(t1);
    assert(id1 > 0);

    // 2. Query All Tracks
    auto allTracks = db.getAllTracks();
    assert(allTracks.size() == 1);
    assert(allTracks[0].title == "High Resolution Symphony");
    assert(allTracks[0].sampleRate == 96000);

    // 3. Search Tracks
    auto searchRes = db.searchTracks("Philharmonic");
    assert(searchRes.size() == 1);
    auto emptyRes = db.searchTracks("Nonexistent Band");
    assert(emptyRes.empty());

    // 4. Favorite Toggle
    assert(!allTracks[0].isFavorite);
    db.setTrackFavorite(id1, true);
    auto favs = db.getFavoriteTracks();
    assert(favs.size() == 1);
    assert(favs[0].isFavorite);

    // 5. Play Count Increment
    db.incrementPlayCount(id1);
    auto mostPlayed = db.getMostPlayedTracks();
    assert(!mostPlayed.empty());
    assert(mostPlayed[0].playCount == 1);

    // 6. Playlists
    int64_t plId = db.createPlaylist("Audiophile Master Collection");
    assert(plId > 0);

    bool addOk = db.addTrackToPlaylist(plId, id1);
    assert(addOk);

    auto plTracks = db.getPlaylistTracks(plId);
    assert(plTracks.size() == 1);
    assert(plTracks[0].id == id1);

    auto allPl = db.getAllPlaylists();
    assert(allPl.size() == 1);
    assert(allPl[0].name == "Audiophile Master Collection");
    assert(allPl[0].trackCount == 1);

    // 7. Preferences
    db.setPreference("theme_mode", "Dark");
    assert(db.getPreference("theme_mode") == "Dark");

    std::cout << "  [PASS] SQLite schema migrations, track CRUD, playlists, and preferences" << std::endl;
    std::cout << "All Database Unit Tests PASSED successfully.\n" << std::endl;
}
