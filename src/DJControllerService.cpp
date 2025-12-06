#include "DJControllerService.h"
#include "MP3Track.h"
#include "WAVTrack.h"
#include <iostream>
#include <memory>

DJControllerService::DJControllerService(size_t cache_size)
    : cache(cache_size) {}
/**
 * TODO: Implement loadTrackToCache method
 */
int DJControllerService::loadTrackToCache(AudioTrack& track) {
    // Your implementation here 
    // (a) hit case
    if(cache.contains(track.get_title())){
        cache.get(track.get_title());
        return 1;
    }
    // (c) miss case
    PointerWrapper<AudioTrack> cloned = track.clone();
    if(!cloned){
        std::cerr << "[Error] clone() returned null for track: " << track.get_title() << "\n";
    return -2;  
    }
    AudioTrack* track_copy = cloned.release();
    track_copy->load();
    track_copy->analyze_beatgrid();


    PointerWrapper<AudioTrack> prepared(track_copy);
    bool evicted = cache.put(std::move(prepared));
    if(evicted){
        return -1;
    }
    else{
        return 0;
    }
    
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

/**
 * TODO: Implement getTrackFromCache method
 */
AudioTrack* DJControllerService::getTrackFromCache(const std::string& track_title) {
    // Your implementation here
      return cache.get(track_title);
}
