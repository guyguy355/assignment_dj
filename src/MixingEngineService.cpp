#include "MixingEngineService.h"
#include <iostream>
#include <memory>


/**
 * TODO: Implement MixingEngineService constructor
 */
MixingEngineService::MixingEngineService()
    : decks(), active_deck(1), auto_sync(false), bpm_tolerance(0)
{
    // Your implementation here
    decks[0] = nullptr;
    decks[1] = nullptr;

     std::cout << "[MixingEngineService] Initialized with 2 empty decks." << std::endl;
}

/**
 * TODO: Implement MixingEngineService destructor
 */
MixingEngineService::~MixingEngineService() {
    // Your implementation here
    std::cout << "[MixingEngineService] Cleaning up decks..." << std::endl;

    for (int i = 0; i < 2; ++i) {
        if (decks[i] != nullptr) {
            delete decks[i];     
            decks[i] = nullptr;  
        }
    }
}


/**
 * TODO: Implement loadTrackToDeck method
 * @param track: Reference to the track to be loaded
 * @return: Index of the deck where track was loaded, or -1 on failure
 */
int MixingEngineService::loadTrackToDeck(const AudioTrack& track) {
    // Your implementation here
    // (a)
    std::cout << "\n=== Loading Track to Deck ===" << std::endl;

    //(b)
    PointerWrapper<AudioTrack> cloned = track.clone();
    //(c)
    if (!cloned) {
    std::cerr << "[ERROR] Track: \"" << track.get_title() << "\" failed to clone" << std::endl;
    return -1;    
    }
    
    // for (j)
    bool had_previous = (decks[active_deck] != nullptr);

    // (d)
    int target;
    if(decks[active_deck] == nullptr){
        target = active_deck;
    }
    else{
        target = 1- active_deck;
    }

    // (e)
    std::cout << "[Deck Switch] Target deck: " << target << std::endl;

    //(f)
    if (decks[target] != nullptr) {
    delete decks[target];       
    decks[target] = nullptr;
    }
    
    // (g)
    cloned->load();              
    cloned->analyze_beatgrid();

    // (h)
    if (decks[active_deck] != nullptr && auto_sync) {
        if (!can_mix_tracks(cloned)) {
            sync_bpm(cloned);
        }
    }

    //(i)
    AudioTrack* prepared = cloned.release();
    decks[target] = prepared;
    std::cout << "[Load Complete] '" << prepared->get_title() << "' is now loaded on deck " << target << std::endl;
    
    // (j)
   if (had_previous && active_deck != target) {
        std::cout << "[Unload] Unloading previous deck " << active_deck << " (" << decks[active_deck]->get_title() << ")" << std::endl;
        delete decks[active_deck];
        decks[active_deck] = nullptr;
    }
    
    // (k)
    active_deck = target;

    //(i)
    std::cout << "[Active Deck] Switched to deck " << active_deck << std::endl;

    //(m)
    return target; 
}

/**
 * @brief Display current deck status
 */
void MixingEngineService::displayDeckStatus() const {
    std::cout << "\n=== Deck Status ===\n";
    for (size_t i = 0; i < 2; ++i) {
        if (decks[i])
            std::cout << "Deck " << i << ": " << decks[i]->get_title() << "\n";
        else
            std::cout << "Deck " << i << ": [EMPTY]\n";
    }
    std::cout << "Active Deck: " << active_deck << "\n";
    std::cout << "===================\n";
}

/**
 * TODO: Implement can_mix_tracks method
 * 
 * Check if two tracks can be mixed based on BPM difference.
 * 
 * @param track: Track to check for mixing compatibility
 * @return: true if BPM difference <= tolerance, false otherwise
 */
bool MixingEngineService::can_mix_tracks(const PointerWrapper<AudioTrack>& track) const {
    // Your implementation here
    // (1)
    AudioTrack* active_track = decks[active_deck];
    if (active_track == nullptr) {
        return false;  
    }
    // (2)
    AudioTrack* new_track = track.get();
    if (new_track == nullptr) {
        return false;
    }
    // (3)
    int active_bpm = active_track->get_bpm();
    int new_bpm = new_track->get_bpm();
    // (4)
    int diff = active_bpm - new_bpm;
    if (diff < 0 ){
        diff = diff * -1;
    }

    return diff<= bpm_tolerance; 
}

/**
 * TODO: Implement sync_bpm method
 * @param track: Track to synchronize with active deck
 */
void MixingEngineService::sync_bpm(const PointerWrapper<AudioTrack>& track) const {
    // Your implementation here
    // (1)
    AudioTrack* active_track = decks[active_deck];
    AudioTrack* new_track    = track.get();

    if (!active_track || !new_track) {
        return; 
    }
    //(2)
    int original_bpm = new_track->get_bpm();
    int active_bpm   = active_track->get_bpm();
    //(3)
    int avg_bpm = (original_bpm + active_bpm) / 2;
    new_track->set_bpm(avg_bpm);
    //(4)
     std::cout << "[Sync BPM] Syncing BPM from " << original_bpm << " to " << avg_bpm << std::endl;
}
