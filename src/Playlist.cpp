#include "Playlist.h"
#include "AudioTrack.h"
#include <iostream>
#include <algorithm>
Playlist::Playlist(const std::string& name)
    : head(nullptr), playlist_name(name), track_count(0) {
    std::cout << "Created playlist: " << name << std::endl;
}
Playlist::~Playlist() {
    clear();
}

void Playlist::clear() {
    while (head) {
        PlaylistNode* next = head->next;
        delete head->track;
        delete head;
        head = next;
    }
    track_count = 0;
}

Playlist::Playlist(const Playlist& other)
    : head(nullptr), playlist_name(other.playlist_name), track_count(0) {
    try {
        PlaylistNode* current = other.head;
        while (current) {
            PointerWrapper<AudioTrack> cloned = current->track->clone();
            if (!cloned) throw std::runtime_error("Track clone failed");
            add_track(cloned.release());
            current = current->next;
        }
    } catch (...) {
        clear();
        throw;
    }
}

Playlist& Playlist::operator=(const Playlist& other) {
    if (this != &other) {
        Playlist copy(other);
        swap(copy);
    }
    return *this;
}

Playlist::Playlist(Playlist&& other) noexcept
    : head(other.head), playlist_name(std::move(other.playlist_name)), track_count(other.track_count) {
    other.head = nullptr;
    other.track_count = 0;
}

Playlist& Playlist::operator=(Playlist&& other) noexcept {
    if (this != &other) {
        clear();
        head = other.head;
        playlist_name = std::move(other.playlist_name);
        track_count = other.track_count;
        other.head = nullptr;
        other.track_count = 0;
    }
    return *this;
}

void Playlist::swap(Playlist& other) noexcept {
    std::swap(head, other.head);
    playlist_name.swap(other.playlist_name);
    std::swap(track_count, other.track_count);
}

void Playlist::add_track(AudioTrack* track) {
    if (!track) return;
    PointerWrapper<AudioTrack> owned(track);
    PlaylistNode* new_node = new PlaylistNode(owned.get());
    owned.release();
    if (!head) {
        head = new_node;
    } else {
        PlaylistNode* current = head;
        while (current->next) current = current->next;
        current->next = new_node;
    }
    ++track_count;
}

void Playlist::remove_track(const std::string& title) {
    PlaylistNode* current = head;
    PlaylistNode* prev = nullptr;

    // Find the track to remove
    while (current && current->track->get_title() != title) {
        prev = current;
        current = current->next;
    }

    if (current) {
        // Remove from linked list
        if (prev) {
            prev->next = current->next;
        } else {
            head = current->next;
        }
        // playlist does own the AudioTrack*
        delete current->track;
        delete current;

        track_count--;
        std::cout << "Removed '" << title << "' from playlist" << std::endl;

    } else {
        std::cout << "Track '" << title << "' not found in playlist" << std::endl;
    }
}

void Playlist::display() const {
    std::cout << "\n=== Playlist: " << playlist_name << " ===" << std::endl;
    std::cout << "Track count: " << track_count << std::endl;

    PlaylistNode* current = head;
    int index = 1;

    while (current) {
        std::vector<std::string> artists = current->track->get_artists();
        std::string artist_list;

        std::for_each(artists.begin(), artists.end(), [&](const std::string& artist) {
            if (!artist_list.empty()) {
                artist_list += ", ";
            }
            artist_list += artist;
        });

        AudioTrack* track = current->track;
        std::cout << index << ". " << track->get_title()
                  << " by " << artist_list
                  << " (" << track->get_duration() << "s, "
                  << track->get_bpm() << " BPM)" << std::endl;
        current = current->next;
        index++;
    }

    if (track_count == 0) {
        std::cout << "(Empty playlist)" << std::endl;
    }
    std::cout << "========================\n" << std::endl;
}

AudioTrack* Playlist::find_track(const std::string& title) const {
    PlaylistNode* current = head;

    while (current) {
        if (current->track->get_title() == title) {
            return current->track;
        }
        current = current->next;
    }

    return nullptr;
}

int Playlist::get_total_duration() const {
    int total = 0;
    PlaylistNode* current = head;

    while (current) {
        total += current->track->get_duration();
        current = current->next;
    }

    return total;
}

std::vector<AudioTrack*> Playlist::getTracks() const {
    std::vector<AudioTrack*> tracks;
    PlaylistNode* current = head;
    while (current) {
        if (current->track)
            tracks.push_back(current->track);
        current = current->next;
    }
    return tracks;
}
