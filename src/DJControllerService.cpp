#include "DJControllerService.h"
#include "MP3Track.h"
#include "WAVTrack.h"
#include <iostream>
#include <memory>

DJControllerService::DJControllerService(size_t cache_size)
    : cache(cache_size) {}

int DJControllerService::loadTrackToCache(AudioTrack& track) {
    // (a) hit case
    if (cache.get(track.get_title())) {
        return 1;
    }
    PointerWrapper<AudioTrack> cloned = track.clone();
    if (!cloned) {
        return -2;
    }
    cloned->load();
    cloned->analyze_beatgrid();
    bool evicted = cache.put(std::move(cloned));
    return evicted ? -1 : 0;
}

void DJControllerService::set_cache_size(size_t new_size) {
    cache.set_capacity(new_size);
}
//implemented
void DJControllerService::displayCacheStatus() const {
    std::cout << "\n=== Cache Status ===\n";
    cache.displayStatus();
    std::cout << "====================\n";
}

AudioTrack* DJControllerService::getTrackFromCache(const std::string& track_title) {
      return cache.get(track_title);
}
