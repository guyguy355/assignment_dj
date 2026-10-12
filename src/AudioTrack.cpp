#include "AudioTrack.h"
#include <iostream>
#include <cstring>
#include <random>

AudioTrack::AudioTrack(const std::string& title, const std::vector<std::string>& artists,
                      int duration, int bpm, size_t waveform_samples)
    : title(title), artists(artists), duration_seconds(duration), bpm(bpm),
        waveform_data(nullptr),waveform_size(waveform_samples) {

    if (title.empty() || duration <= 0 || bpm <= 0 || bpm > 1000)
        throw std::invalid_argument("Track needs a title, positive duration and BPM in 1..1000");

    // Allocate memory for waveform analysis

    // Generate some dummy waveform data for testing
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(-1.0, 1.0);
    waveform_data = new double[waveform_size];

    for (size_t i = 0; i < waveform_size; ++i) {
        waveform_data[i] = dis(gen);
    }
    #ifdef DEBUG
    std::cout << "AudioTrack created: " << title << " by " << std::endl;
    for (const auto& artist : artists) {
        std::cout << artist << " ";
    }
    std::cout << std::endl;
    #endif
}

AudioTrack::~AudioTrack() {
    #ifdef DEBUG
    std::cout << "AudioTrack destructor called for: " << title << std::endl;
    #endif
    delete []waveform_data;
    waveform_data = nullptr;
    waveform_size = 0;
}

AudioTrack::AudioTrack(const AudioTrack& other)
:title(other.title), artists(other.artists),duration_seconds(other.duration_seconds),
    bpm(other.bpm),waveform_data(nullptr), waveform_size(other.waveform_size)
{
    #ifdef DEBUG
    std::cout << "AudioTrack copy constructor called for: " << other.title << std::endl;
    #endif
   if (waveform_size > 0 && other.waveform_data) {
        waveform_data = new double[waveform_size];
        for (size_t i = 0; i < waveform_size; ++i) {
            waveform_data[i] = other.waveform_data[i];
        }
    }
}

AudioTrack& AudioTrack::operator=(const AudioTrack& other) {
    #ifdef DEBUG
    std::cout << "AudioTrack copy assignment called for: " << other.title << std::endl;
    #endif

    if (this == &other) return *this;

    std::string new_title = other.title;
    std::vector<std::string> new_artists = other.artists;
    double* new_waveform_data = nullptr;
    if (other.waveform_size > 0) {
        new_waveform_data = new double[other.waveform_size];
        for (size_t i = 0; i < other.waveform_size; ++i) {
            new_waveform_data[i] = other.waveform_data[i];
        }
    }
    title.swap(new_title);
    artists.swap(new_artists);
    delete[] waveform_data;
    waveform_data = new_waveform_data;
    waveform_size = other.waveform_size;
    duration_seconds = other.duration_seconds;
    bpm = other.bpm;
    return *this;
}

AudioTrack::AudioTrack(AudioTrack&& other) noexcept : title(std::move(other.title)),
      artists(std::move(other.artists)),duration_seconds(other.duration_seconds),bpm(other.bpm),
      waveform_data(other.waveform_data), waveform_size(other.waveform_size) {
    #ifdef DEBUG
    std::cout << "AudioTrack move constructor called for: " << other.title << std::endl;
    #endif
    other.waveform_data = nullptr;
    other.waveform_size = 0;
}

AudioTrack& AudioTrack::operator=(AudioTrack&& other) noexcept {

    #ifdef DEBUG
    std::cout << "AudioTrack move assignment called for: " << other.title << std::endl;
    #endif
    if (this != &other) {
        // 2. Safely release the OLD resources owned by *this
        delete[] waveform_data;

        // 3. Move (transfer) all members from 'other' to *this.
        // The assignment operator for std::string/std::vector handles the move internally.
        title = std::move(other.title);
        artists = std::move(other.artists);

        duration_seconds = other.duration_seconds;
        bpm = other.bpm;
        waveform_size = other.waveform_size;

        // Transfer the raw pointer ownership
        waveform_data = other.waveform_data;

        // 4. Nullify the source object's state
        // This makes 'other' safe to destroy (its destructor won't delete the memory we now own).
        other.waveform_data = nullptr;
        other.waveform_size = 0;
    }
    return *this;
}

void AudioTrack::get_waveform_copy(double* buffer, size_t buffer_size) const {
    if (buffer && waveform_data && buffer_size <= waveform_size) {
        std::memcpy(buffer, waveform_data, buffer_size * sizeof(double));
    }
}
