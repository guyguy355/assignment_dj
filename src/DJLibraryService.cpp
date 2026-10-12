#include "DJLibraryService.h"
#include "SessionFileParser.h"
#include "MP3Track.h"
#include "WAVTrack.h"
#include <iostream>
#include <memory>

DJLibraryService::DJLibraryService(const Playlist& playlist)
    : playlist(playlist), library() {}
/**
 * @brief Load a playlist from track indices referencing the library
 * @param library_tracks Vector of track info from config
 */

 DJLibraryService::~DJLibraryService() {
    for (AudioTrack* track : library) {
        delete track;
    }
    library.clear();
}

void DJLibraryService::buildLibrary(const std::vector<SessionConfig::TrackInfo>& library_tracks) {
     for (AudioTrack* t : library) {
        delete t;
    }
    library.clear();

    for (const auto& info : library_tracks) {

        AudioTrack* track = nullptr;

        if (info.type == "MP3") {
            // extra_param1 = bitrate, extra_param2 = has_tags (0/1)
            bool has_tags = (info.extra_param2 != 0);

            track = new MP3Track(
                info.title,
                info.artists,
                info.duration_seconds,
                info.bpm,
                info.extra_param1,
                has_tags
            );

            //std::cout << "MP3: MP3Track created: " << info.extra_param1 << " kbps" << std::endl;
        }
        else if (info.type == "WAV") {
            // extra_param1 = sample_rate, extra_param2 = bit_depth

            track = new WAVTrack(
                info.title,
                info.artists,
                info.duration_seconds,
                info.bpm,
                info.extra_param1,  // sample_rate
                info.extra_param2   // bit_depth
            );

            //std::cout << "WAV: WAVTrack created: " << info.extra_param1 << "Hz/" << info.extra_param2 << "bit" << std::endl;
        }

        PointerWrapper<AudioTrack> owned(track);
        if (!owned) throw std::invalid_argument("Unsupported track type");
        library.push_back(owned.get());
        owned.release();
    }

    // summery log
    std::cout << "[INFO] Track library built: " << library.size() << " tracks loaded" << std::endl;
}

/**
 * @brief Display the current state of the DJ library playlist
 *
 */
void DJLibraryService::displayLibrary() const {
    std::cout << "=== DJ Library Playlist: "
              << playlist.get_name() << " ===" << std::endl;

    if (playlist.is_empty()) {
        std::cout << "[INFO] Playlist is empty.\n";
        return;
    }

    // Let Playlist handle printing all track info
    playlist.display();

    std::cout << "Total duration: " << playlist.get_total_duration() << " seconds" << std::endl;
}

/**
 * @brief Get a reference to the current playlist
 *
 * @return Playlist&
 */
Playlist& DJLibraryService::getPlaylist() {
    return playlist;
}

AudioTrack* DJLibraryService::findTrack(const std::string& track_title) {
    return playlist.find_track(track_title);
}

void DJLibraryService::loadPlaylistFromIndices(const std::string& playlist_name,
                                               const std::vector<int>& track_indices) {

    std::cout << "[INFO] Loading playlist: " << playlist_name << std::endl;

    Playlist playlist(playlist_name);

    int added_count = 0;

    for(int index : track_indices){
        if (index < 1 || index > static_cast<int>(library.size())) {
            std::cout << "[WARNING] Invalid track index: " << index << " in playlist '" << playlist_name << "'" << std::endl;
            continue;
        }
        AudioTrack* original = library[index - 1];

        PointerWrapper<AudioTrack> cloned = original->clone();

        if (!cloned) {
            std::cerr << "[ERROR] Track: \"" << original->get_title()
                      << "\" failed to clone for playlist '" << playlist_name << "'" << std::endl;
            continue;
        }

        AudioTrack* clone_ptr = cloned.get();
        clone_ptr->load();
        clone_ptr->analyze_beatgrid();

        playlist.add_track(cloned.release());

        added_count++;

        //std::cout << "Added '" << clone_ptr->get_title() << "' to playlist '" << playlist_name << "'" << std::endl;
    }
    std::cout << "[INFO] Playlist loaded: " << playlist_name << " (" << added_count << " tracks)" << std::endl;
    this->playlist = std::move(playlist);

}

std::vector<std::string> DJLibraryService::getTrackTitles() const {
    std::vector<std::string> titles;
    //titles.reserve(playlist.getTracks().size());

    //for (AudioTrack* track : playlist.getTracks()) {
        //if (track != nullptr) {
            //titles.push_back(track->get_title());
        //}
    //}
    const auto& tracks = playlist.getTracks();
    titles.reserve(tracks.size());
    for (auto it = tracks.rbegin(); it != tracks.rend(); ++it) {
        AudioTrack* track = *it;
        if (track != nullptr) {
            titles.push_back(track->get_title());
        }
    }

    return titles;
}
