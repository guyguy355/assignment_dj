#include "Playlist.h"
#include "AudioTrack.h"
#include <iostream>
#include <algorithm>
Playlist::Playlist(const std::string& name) 
    : head(nullptr), playlist_name(name), track_count(0) {
    std::cout << "Created playlist: " << name << std::endl;
}
// TODO: Fix memory leaks!
// Students must fix this in Phase 1
Playlist::~Playlist() {
    #ifdef DEBUG
    std::cout << "Destroying playlist: " << playlist_name << std::endl;
    #endif

    PlaylistNode* current = head;

    while (current != nullptr) {
        PlaylistNode* next = current->next;

        // ownimg model: PlaylistNode own the AudioTrack* so it's he's resposability to delete it.
        
        delete current->track;
        delete current;

        current = next;
    }

    head = nullptr;
    track_count = 0;
}

// we added a copy constructor
Playlist::Playlist(const Playlist& other)
    : head(nullptr),
      playlist_name(other.playlist_name),
      track_count(0)
{
    PlaylistNode* current_other = other.head;
    PlaylistNode* prev_new = nullptr;

    while (current_other) {
        // 1. משכפלים פולימורפית את ה־AudioTrack
        PointerWrapper<AudioTrack> cloned = current_other->track->clone();
        AudioTrack* track_copy = cloned.release();   // הבעלות עוברת אלינו

        // 2. יוצרים node חדש ב־playlist
        PlaylistNode* new_node = new PlaylistNode(track_copy);

        if (!head) {
            head = new_node;
        } else {
            prev_new->next = new_node;
        }

        prev_new = new_node;
        current_other = current_other->next;
        ++track_count;
    }
}


// we added an copy operator
Playlist& Playlist::operator=(const Playlist& other) {
    if (this == &other) {
        return *this; // הגנה מהעתקה עצמית
    }

    // 1. לנקות את הפלייליסט הנוכחי (כמו ב-destructor)
    PlaylistNode* current = head;
    while (current) {
        PlaylistNode* next = current->next;

        delete current->track;  // ה-Playlist הוא ה-owner של ה-AudioTrack*
        delete current;

        current = next;
    }
    head = nullptr;
    track_count = 0;

    // 2. להעתיק את שם הפלייליסט
    playlist_name = other.playlist_name;

    // 3. לבנות מחדש רשימה ע"י clone פולימורפי של כל ה-tracks
    PlaylistNode* prev_new = nullptr;
    PlaylistNode* current_other = other.head;

    while (current_other) {
        // פולימורפי: קורא ל-clone של המחלקה הנגזרת המתאימה
        PointerWrapper<AudioTrack> cloned = current_other->track->clone();
        AudioTrack* track_copy = cloned.release();  // הבעלות עוברת אלינו

        PlaylistNode* new_node = new PlaylistNode(track_copy);

        if (!head) {
            head = new_node;
        } else {
            prev_new->next = new_node;
        }

        prev_new = new_node;
        current_other = current_other->next;
        ++track_count;
    }

    return *this;
}


void Playlist::add_track(AudioTrack* track) {
    if (!track) {
        std::cout << "[Error] Cannot add null track to playlist" << std::endl;
        return;
    }

    // Create new node - this allocates memory!
    PlaylistNode* new_node = new PlaylistNode(track);

    // Add to front of list
    new_node->next = head;
    head = new_node;
    track_count++;

    std::cout << "Added '" << track->get_title() << "' to playlist '" 
              << playlist_name << "'" << std::endl;
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