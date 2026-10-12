#pragma once
#include "SessionFileParser.h"

// Suggests a tempo route and replays requests through the real controller cache.
class SetJourney {
public:
    static std::vector<int> suggest_order(const SessionConfig& config, const std::vector<int>& order);
    static int total_jump(const SessionConfig& config, const std::vector<int>& order);
    static void write_report(const SessionConfig& config, const std::string& playlist_name,
                             const std::string& output_path);
};
