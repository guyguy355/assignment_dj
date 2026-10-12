#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>

/**
 * @brief Configuration data parsed from DJ session config files
 */
struct SessionConfig {
    std::string app_name;
    std::string version;

    // Library tracks from config file
    struct TrackInfo {
        std::string type;        // MP3 or WAV
        std::string title;
        std::vector<std::string> artists;  // Changed to vector for multiple artists
        int duration_seconds;
        int bpm;
        int extra_param1;        // bitrate for MP3, sample_rate for WAV
        int extra_param2;        // has_tags for MP3, bit_depth for WAV

        TrackInfo()
            : type(""),
              title(""),
              artists(),
              duration_seconds(0),
              bpm(0),
              extra_param1(0),
              extra_param2(0) {}
    };

    std::vector<TrackInfo> library_tracks;

    // Cache settings
    int controller_cache_size;

    // Mixing settings
    int default_crossfade_time;
    int bpm_tolerance;
    bool auto_sync;

    // Playlists - name mapped to list of track indices
    std::map<std::string, std::vector<int>> playlists;
    SessionConfig()
        : app_name(""),
          version(""),
          library_tracks(),
          controller_cache_size(8),
          default_crossfade_time(5),
          bpm_tolerance(10),
          auto_sync(true),
          playlists() {}
};

// Parses session settings, library metadata and playlist indices.
class SessionFileParser {
public:
    /**
     * @brief Parse a DJ session configuration file
     * @param config_path Path to the .txt configuration file
     * @param config Output structure for parsed data
     * @return true if parsing successful, false on error
     *
     * Expected format:
     * # Comments start with #
     * app_name=DJ Track Library Manager
     * version=2.0
     * library_track_1=MP3,title,{artist1;artist2;},duration,bpm,bitrate,has_tags
     * library_track_2=WAV,title,{artist1;artist2;},duration,bpm,sample_rate,bit_depth
     * controller_cache_size=8
     * bpm_tolerance=10
     * auto_sync=true
     * playlistname=1,2,3
     */
    static bool parse_config_file(const std::string& config_path, SessionConfig& config);

    /**
     * @brief Extract playlist name from file path
     * @param playlist_path Full path to playlist file
     * @return Extracted name (filename without extension)
     */
    static std::string extract_playlist_name(const std::string& playlist_path);

    /**
     * @brief Validate track format line
     * @param line Line to validate
     * @return true if line matches expected track format
     */
    static bool validate_track_format(const std::string& line);

private:
    /**
     * @brief Split a string by delimiter
     * @param str String to split
     * @param delimiter Character to split on
     * @return Vector of split parts
     */
    static std::vector<std::string> split_string(const std::string& str, char delimiter);

    /**
     * @brief Trim whitespace from string
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string trim_string(const std::string& str);

    /**
     * @brief Parse boolean value from string
     * @param str String containing "true" or "false"
     * @return Parsed boolean value
     */
    static bool parse_bool(const std::string& str);

    /**
     * @brief Check if line is a comment (starts with #)
     * @param line Line to check
     * @return true if comment line
     */
    static bool is_comment_line(const std::string& line);

    /**
     * @brief Parse key=value pair from configuration line
     * @param line Configuration line
     * @param key Output key
     * @param value Output value
     * @return true if parsing successful
     */
    static bool parse_key_value(const std::string& line, std::string& key, std::string& value);

    /**
     * @brief Parse library_track line from config
     * @param line library_track configuration line
     * @param track_info Output track information
     * @return true if parsing successful
     */
    static bool parse_library_track(const std::string& line, SessionConfig::TrackInfo& track_info);

    /**
     * @brief Parse artist list from {artist1;artist2;...} format
     * @param artist_str String containing artists in curly braces
     * @return Vector of artist names
     */
    static std::vector<std::string> parse_artist_list(const std::string& artist_str);

    /**
     * @brief Parse playlist line from config (playlist_name=1,2,3)
     * @param line Playlist line from config
     * @param playlist_name Output playlist name
     * @param track_indices Output vector of track indices
     * @return true if parsing successful
     */
    static bool parse_playlist_line(const std::string& line, std::string& playlist_name, std::vector<int>& track_indices);

};
