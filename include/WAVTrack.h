#ifndef WAVTRACK_H
#define WAVTRACK_H

#include "AudioTrack.h"

class WAVTrack : public AudioTrack {
private:
    int sample_rate;    // Samples per second: 44100 (CD), 48000 (pro), 96000+ (hi-res)
    int bit_depth;      // Bits per sample: 16 (CD), 24 (pro), 32 (float)

public:
    /**
     * Constructor for WAVTrack
     */
    WAVTrack(const std::string& title, const std::vector<std::string>& artists,
             int duration, int bpm, int sample_rate, int bit_depth);

    void load() override;

    void analyze_beatgrid() override;

    double get_quality_score() const override;

    PointerWrapper<AudioTrack> clone() const override;

    // Getters
    int get_sample_rate() const { return sample_rate; }
    int get_bit_depth() const { return bit_depth; }
};

#endif // WAVTRACK_H
