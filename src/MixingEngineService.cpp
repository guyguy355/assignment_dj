#include "MixingEngineService.h"
#include <iostream>
#include <memory>

MixingEngineService::MixingEngineService()
    : decks(), active_deck(1), auto_sync(false), bpm_tolerance(0)
{
    decks[0] = nullptr;
    decks[1] = nullptr;

     std::cout << "[MixingEngineService] Initialized with 2 empty decks." << std::endl;
}

MixingEngineService::~MixingEngineService() {
    std::cout << "[MixingEngineService] Cleaning up decks..." << std::endl;

    for (int i = 0; i < 2; ++i) {
        if (decks[i] != nullptr) {
            delete decks[i];
            decks[i] = nullptr;
        }
    }
}

int MixingEngineService::loadTrackToDeck(const AudioTrack& track) {

    std::cout << "\n=== Loading Track to Deck ===" << std::endl;

    PointerWrapper<AudioTrack> cloned = track.clone();

    if (!cloned) {
    std::cerr << "[ERROR] Track: \"" << track.get_title() << "\" failed to clone" << std::endl;
    return -1;
    }

    // for (j)
    bool had_previous = (decks[active_deck] != nullptr);

    int target;
    if(decks[active_deck] == nullptr){
        target = active_deck;
    }
    else{
        target = 1- active_deck;
    }

    std::cout << "[Deck Switch] Target deck: " << target << std::endl;

    if (decks[target] != nullptr) {
    delete decks[target];
    decks[target] = nullptr;
    }

    cloned->load();
    cloned->analyze_beatgrid();

    if (decks[active_deck] != nullptr && auto_sync) {
        if (!can_mix_tracks(cloned)) {
            sync_bpm(cloned);
        }
    }

    AudioTrack* prepared = cloned.release();
    decks[target] = prepared;
    std::cout << "[Load Complete] '" << prepared->get_title() << "' is now loaded on deck " << target << std::endl;

   if (had_previous && active_deck != static_cast<std::size_t>(target)) {
        std::cout << "[Unload] Unloading previous deck " << active_deck << " (" << decks[active_deck]->get_title() << ")" << std::endl;
        delete decks[active_deck];
        decks[active_deck] = nullptr;
    }

    active_deck = target;

    std::cout << "[Active Deck] Switched to deck " << active_deck << std::endl;

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

bool MixingEngineService::can_mix_tracks(const PointerWrapper<AudioTrack>& track) const {

    AudioTrack* active_track = decks[active_deck];
    if (active_track == nullptr) {
        return false;
    }

    AudioTrack* new_track = track.get();
    if (new_track == nullptr) {
        return false;
    }

    int active_bpm = active_track->get_bpm();
    int new_bpm = new_track->get_bpm();

    int diff = active_bpm - new_bpm;
    if (diff < 0 ){
        diff = diff * -1;
    }

    return diff<= bpm_tolerance;
}

void MixingEngineService::sync_bpm(const PointerWrapper<AudioTrack>& track) const {

    AudioTrack* active_track = decks[active_deck];
    AudioTrack* new_track    = track.get();

    if (!active_track || !new_track) {
        return;
    }

    int original_bpm = new_track->get_bpm();
    int active_bpm   = active_track->get_bpm();

    int avg_bpm = active_bpm;
    new_track->set_bpm(avg_bpm);

     std::cout << "[Sync BPM] Syncing BPM from " << original_bpm << " to " << avg_bpm << std::endl;
}
