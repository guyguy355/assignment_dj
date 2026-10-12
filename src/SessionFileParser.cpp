#include "SessionFileParser.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <set>
#include <stdexcept>
#include <cctype>

namespace {
int parse_number(const std::string& text, int minimum, int maximum) {
    size_t end = 0;
    int value = std::stoi(text, &end);
    if (end != text.size() || value < minimum || value > maximum)
        throw std::invalid_argument("Invalid number: " + text);
    return value;
}
}

bool SessionFileParser::parse_config_file(const std::string& config_path, SessionConfig& config) {
    std::ifstream file(config_path);
    if (!file) {
        std::cerr << "Cannot open config: " << config_path << std::endl;
        return false;
    }
    SessionConfig parsed;
    std::map<int, SessionConfig::TrackInfo> tracks;
    std::set<std::string> keys;
    std::set<std::string> titles;
    std::string line;
    int line_number = 0;
    try {
        while (std::getline(file, line)) {
            ++line_number;
            line = trim_string(line);
            if (line.empty() || is_comment_line(line)) continue;
            std::string key, value;
            if (!parse_key_value(line, key, value) || !keys.insert(key).second)
                throw std::invalid_argument("Malformed or duplicate setting");
            if (key == "app_name") parsed.app_name = value;
            else if (key == "version") parsed.version = value;
            else if (key.find("library_track_") == 0) {
                int index = parse_number(key.substr(14), 1, 10000);
                SessionConfig::TrackInfo info;
                if (!parse_library_track(value, info) || !titles.insert(info.title).second)
                    throw std::invalid_argument("Invalid track or duplicate title");
                if (!tracks.insert(std::make_pair(index, info)).second)
                    throw std::invalid_argument("Duplicate track index");
            } else if (key == "controller_cache_size") {
                parsed.controller_cache_size = parse_number(value, 1, 1024);
            } else if (key == "bpm_tolerance") {
                parsed.bpm_tolerance = parse_number(value, 0, 1000);
            } else if (key == "auto_sync") {
                if (value != "true" && value != "false" && value != "1" && value != "0")
                    throw std::invalid_argument("auto_sync must be true/false or 1/0");
                parsed.auto_sync = parse_bool(value);
            } else {
                std::string name;
                std::vector<int> indices;
                if (!parse_playlist_line(line, name, indices))
                    throw std::invalid_argument("Invalid playlist");
                parsed.playlists[name] = indices;
            }
        }
        if (tracks.empty()) throw std::invalid_argument("Library is empty");
        int expected = 1;
        for (const auto& entry : tracks) {
            if (entry.first != expected++) throw std::invalid_argument("Track indices must be contiguous from 1");
            parsed.library_tracks.push_back(entry.second);
        }
        for (const auto& playlist : parsed.playlists) {
            for (int index : playlist.second) {
                if (index > static_cast<int>(parsed.library_tracks.size()))
                    throw std::invalid_argument("Playlist references a missing track");
            }
        }
        config = std::move(parsed);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Config error near line " << line_number << ": " << e.what() << std::endl;
        return false;
    }
}

std::string SessionFileParser::extract_playlist_name(const std::string& playlist_path) {

    // Find last slash or backslash
    size_t last_slash = playlist_path.find_last_of("/\\");
    std::string filename = (last_slash != std::string::npos) ?
                          playlist_path.substr(last_slash + 1) : playlist_path;

    // Remove .playlist extension
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos != std::string::npos && filename.substr(dot_pos) == ".playlist") {
        filename = filename.substr(0, dot_pos);
    }

    std::replace(filename.begin(), filename.end(), '_', ' ');

    return filename;
}

bool SessionFileParser::validate_track_format(const std::string& line) {
    SessionConfig::TrackInfo info;
    return parse_library_track(line, info);
}

std::vector<std::string> SessionFileParser::split_string(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(trim_string(token));
    }

    if (!str.empty() && str.back() == delimiter) tokens.push_back("");
    return tokens;
}

std::string SessionFileParser::trim_string(const std::string& str) {
    const std::string whitespace = " \t\n\r";

    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }

    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

bool SessionFileParser::parse_bool(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    return (lower_str == "true" || lower_str == "1" || lower_str == "yes");
}

bool SessionFileParser::is_comment_line(const std::string& line) {
    return !line.empty() && line[0] == '#';
}

bool SessionFileParser::parse_key_value(const std::string& line, std::string& key, std::string& value) {
    size_t equals_pos = line.find('=');
    if (equals_pos == std::string::npos) {
        return false;
    }

    key = trim_string(line.substr(0, equals_pos));
    value = trim_string(line.substr(equals_pos + 1));

    return !key.empty();
}

bool SessionFileParser::parse_library_track(const std::string& line, SessionConfig::TrackInfo& track_info) {
    // Expected format: MP3,title,{artist1;artist2;},duration,bpm,bitrate,has_tags
    // or: WAV,title,{artist1;artist2;},duration,bpm,sample_rate,bit_depth

    std::vector<std::string> parts = split_string(line, ',');

    if (parts.size() != 7) {
        return false;
    }

    try {
        track_info.type = parts[0];
        track_info.title = parts[1];
        if (parts[1].empty() || parts[2].size() < 2 || parts[2].front() != '{' || parts[2].back() != '}') return false;

        // Parse artist list from {artist1;artist2;...} format
        track_info.artists = parse_artist_list(parts[2]);

        track_info.duration_seconds = parse_number(parts[3], 1, 86400);
        track_info.bpm = parse_number(parts[4], 1, 1000);
        track_info.extra_param1 = parse_number(parts[5], 1, 384000);  // bitrate or sample_rate
        track_info.extra_param2 = parse_number(parts[6], 0, 64);  // has_tags or bit_depth

        // Validate track type is MP3 or WAV
        if (track_info.type != "MP3" && track_info.type != "WAV") {
            return false;
        }

        if (track_info.type == "MP3" && (track_info.extra_param1 > 320 || track_info.extra_param2 > 1)) return false;
        if (track_info.type == "WAV" && track_info.extra_param2 != 8 && track_info.extra_param2 != 16 && track_info.extra_param2 != 24 && track_info.extra_param2 != 32) return false;
        return true;

    } catch (const std::exception& e) {
        return false;
    }
}

std::vector<std::string> SessionFileParser::parse_artist_list(const std::string& artist_str) {
    std::vector<std::string> artists;
    std::string cleaned = trim_string(artist_str);

    // Remove curly braces
    if (cleaned.length() >= 2 && cleaned.front() == '{' && cleaned.back() == '}') {
        cleaned = cleaned.substr(1, cleaned.length() - 2);
    }

    // Split by semicolon
    std::vector<std::string> artist_parts = split_string(cleaned, ';');
    for (const auto& artist : artist_parts) {
        std::string trimmed = trim_string(artist);
        if (!trimmed.empty()) {
            artists.push_back(trimmed);
        }
    }

    // If no artists found, add a default
    if (artists.empty()) {
        artists.push_back("Unknown Artist");
    }

    return artists;
}

bool SessionFileParser::parse_playlist_line(const std::string& line, std::string& playlist_name, std::vector<int>& track_indices) {
    // Expected format: playlist_name=1,2,3,4
    std::string key, value;
    if (!parse_key_value(line, key, value)) {
        return false;
    }

    playlist_name = key;
    track_indices.clear();

    // Parse comma-separated indices
    std::vector<std::string> index_strs = split_string(value, ',');
    for (const auto& idx_str : index_strs) {
        try {
            int idx = parse_number(trim_string(idx_str), 1, 10000);
            track_indices.push_back(idx);
        } catch (const std::exception& e) {
            return false;

        }
    }

    return !track_indices.empty();
}
