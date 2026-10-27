#include "audio/PlaybackQueue.hpp"
#include <cassert>
#include <iostream>

void testPlaybackQueue() {
    using namespace soniq::audio;

    std::cout << "\n=== Running PlaybackQueue Unit Tests ===" << std::endl;

    PlaybackQueue queue;
    assert(queue.empty());

    // 1. Append items
    QueueItem item1{0, "track1.flac", "Track 1", "Artist A", "Album X", 180.0};
    QueueItem item2{0, "track2.flac", "Track 2", "Artist A", "Album X", 200.0};
    QueueItem item3{0, "track3.flac", "Track 3", "Artist B", "Album Y", 220.0};

    queue.append(item1);
    queue.append(item2);
    queue.append(item3);

    assert(queue.size() == 3);
    assert(queue.currentItem()->title == "Track 1");

    // 2. Navigation
    assert(queue.hasNext());
    auto nextItem = queue.next();
    assert(nextItem.has_value());
    assert(nextItem->title == "Track 2");
    assert(queue.currentIndex() == 1);

    auto nextItem2 = queue.next();
    assert(nextItem2.has_value());
    assert(nextItem2->title == "Track 3");
    assert(queue.currentIndex() == 2);

    // End of queue with Repeat Off -> hasNext should be false
    queue.setRepeatMode(soniq::core::RepeatMode::Off);
    assert(!queue.hasNext());

    // 3. Repeat Queue mode
    queue.setRepeatMode(soniq::core::RepeatMode::Queue);
    assert(queue.hasNext());
    auto wrapItem = queue.next();
    assert(wrapItem.has_value());
    assert(wrapItem->title == "Track 1");
    assert(queue.currentIndex() == 0);

    // 4. Repeat Track mode
    queue.setRepeatMode(soniq::core::RepeatMode::Track);
    auto repeatTrack = queue.next();
    assert(repeatTrack->title == "Track 1");
    assert(queue.currentIndex() == 0);

    // 5. Reorder / Move
    queue.setRepeatMode(soniq::core::RepeatMode::Off);
    queue.move(0, 2); // Move item 0 ("Track 1") to index 2
    assert(queue.items()[0].title == "Track 2");
    assert(queue.items()[1].title == "Track 3");
    assert(queue.items()[2].title == "Track 1");

    // 6. Insert Next
    QueueItem priorityTrack{0, "priority.flac", "Priority", "Artist C", "Album Z", 150.0};
    queue.setCurrentIndex(0);
    queue.insertNext(priorityTrack);
    assert(queue.items()[1].title == "Priority");

    // 7. Remove
    queue.remove(1);
    assert(queue.items()[1].title == "Track 3");

    std::cout << "  [PASS] Queue operations, repeat modes, reordering, and bounds safety" << std::endl;
    std::cout << "All PlaybackQueue Unit Tests PASSED successfully.\n" << std::endl;
}
