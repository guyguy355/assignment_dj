#include "LRUCache.h"
#include <iostream>
#include <stdexcept>

LRUCache::LRUCache(size_t capacity)
    : slots(capacity), max_size(capacity), access_counter(0) {
    if (capacity == 0) throw std::invalid_argument("Cache capacity must be positive");
}

bool LRUCache::contains(const std::string& track_id) const {
    return findSlot(track_id) != max_size;
}

AudioTrack* LRUCache::get(const std::string& track_id) {
    size_t idx = findSlot(track_id);
    if (idx == max_size) return nullptr;

    return slots[idx].access(++access_counter);
}

bool LRUCache::put(PointerWrapper<AudioTrack> track) {
    if (!track) {
        return false;
    }

    AudioTrack* incoming = track.get();
    for (size_t i = 0; i < max_size; ++i) {
        if (slots[i].isOccupied()) {
            AudioTrack* existing = slots[i].getTrack();
            if (existing && existing->get_title() == incoming->get_title()) {

                slots[i].access(++access_counter);
                return false;
            }
        }
    }

    bool eviction_happened = false;

    size_t empty_index = findEmptySlot();
    if (empty_index == max_size) {

        evictLRU();
        eviction_happened = true;
        empty_index = findEmptySlot();
    }

    slots[empty_index].store(std::move(track), ++access_counter);

    return eviction_happened;
}

bool LRUCache::evictLRU() {
    size_t lru = findLRUSlot();
    if (lru == max_size || !slots[lru].isOccupied()) return false;
    slots[lru].clear();
    return true;
}

size_t LRUCache::size() const {
    size_t count = 0;
    for (const auto& slot : slots) if (slot.isOccupied()) ++count;
    return count;
}

void LRUCache::clear() {
    for (auto& slot : slots) {
        slot.clear();
    }
}

void LRUCache::displayStatus() const {
    std::cout << "\n=== Cache Status ===" << std::endl;
    std::cout << "[LRUCache] Status: " << size() << "/" << max_size << " slots used" << std::endl;
    for (size_t i = 0; i < max_size; ++i) {
        std::cout <<"  Slot " << i << ": ";
        if(slots[i].isOccupied()){
            std::cout << slots[i].getTrack()->get_title()
                      << " (last access: " << slots[i].getLastAccessTime() << ")";
        } else {
            std::cout << "[EMPTY]";
        }
        std::cout << std::endl;
    }
     std::cout << "====================" << std::endl;
}

size_t LRUCache::findSlot(const std::string& track_id) const {
    for (size_t i = 0; i < max_size; ++i) {
        if (slots[i].isOccupied() && slots[i].getTrack()->get_title() == track_id) return i;
    }
    return max_size;

}

size_t LRUCache::findLRUSlot() const {
    size_t index = max_size;
    uint64_t min_time = 0;

     for (size_t i = 0; i < max_size; ++i) {
        if (slots[i].isOccupied()) {
            uint64_t access_time = slots[i].getLastAccessTime();
            if (min_time == 0 || access_time < min_time) {
                min_time = access_time;
                index = i;
            }
        }
        }
    return index;
}

size_t LRUCache::findEmptySlot() const {
    for (size_t i = 0; i < max_size; ++i) {
        if (!slots[i].isOccupied()) return i;
    }
    return max_size;
}

void LRUCache::set_capacity(size_t capacity) {
    if (capacity == 0) throw std::invalid_argument("Cache capacity must be positive");
    if (capacity == max_size) return;
    // Allocate first, so allocation failure leaves this cache unchanged.
    std::vector<CacheSlot> resized(capacity);
    while (size() > capacity) evictLRU();
    size_t next = 0;
    for (size_t i = 0; i < max_size; ++i) {
        if (slots[i].isOccupied()) resized[next++] = std::move(slots[i]);
    }
    slots.swap(resized);
    max_size = capacity;
}
