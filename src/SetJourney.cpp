#include "SetJourney.h"
#include "DJLibraryService.h"
#include "DJControllerService.h"
#include <fstream>
#include <cstdlib>
#include <stdexcept>

namespace {
const SessionConfig::TrackInfo& get_track(const SessionConfig& config, int index) {
    if (index < 1 || index > static_cast<int>(config.library_tracks.size()))
        throw std::invalid_argument("Track index is outside the library");
    return config.library_tracks[index - 1];
}

std::string escape_html(const std::string& value) {
    std::string result;
    for (char c : value) {
        if (c == '&') result += "&amp;";
        else if (c == '<') result += "&lt;";
        else if (c == '>') result += "&gt;";
        else if (c == '"') result += "&quot;";
        else if (c == '\'') result += "&#39;";
        else result += c;
    }
    return result;
}
}

int SetJourney::total_jump(const SessionConfig& config, const std::vector<int>& order) {
    int total = 0;
    for (size_t i = 0; i < order.size(); ++i) {
        const auto& track = get_track(config, order[i]);
        if (i > 0) total += std::abs(track.bpm - get_track(config, order[i - 1]).bpm);
    }
    return total;
}

std::vector<int> SetJourney::suggest_order(const SessionConfig& config, const std::vector<int>& order) {
    // Keep the opening track. Ties keep their original relative order.
    total_jump(config, order);
    if (order.empty()) return order;
    std::vector<int> result(1, order[0]);
    std::vector<bool> used(order.size(), false);
    used[0] = true;
    while (result.size() < order.size()) {
        size_t best = order.size();
        int smallest_jump = 1001;
        int current_bpm = get_track(config, result.back()).bpm;
        for (size_t i = 1; i < order.size(); ++i) {
            if (used[i]) continue;
            int jump = std::abs(current_bpm - get_track(config, order[i]).bpm);
            if (best == order.size() || jump < smallest_jump) {
                best = i;
                smallest_jump = jump;
            }
        }
        used[best] = true;
        result.push_back(order[best]);
    }
    // Greedy is not guaranteed to improve every input.
    if (total_jump(config, result) > total_jump(config, order)) return order;
    return result;
}

void SetJourney::write_report(const SessionConfig& config, const std::string& playlist_name,
                              const std::string& output_path) {
    auto found = config.playlists.find(playlist_name);
    if (found == config.playlists.end() || found->second.empty())
        throw std::invalid_argument("Unknown or empty playlist: " + playlist_name);
    std::vector<int> orders[2] = {found->second, suggest_order(config, found->second)};
    std::ofstream out(output_path);
    if (!out) throw std::runtime_error("Cannot write report: " + output_path);
    out << R"HTML(<!doctype html><html lang="en"><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>DJLab | Set Journey</title>
<style>
*{box-sizing:border-box}body{margin:0;background:#10141e;color:#eef2ff;font:16px/1.6 system-ui,sans-serif}
main{max-width:1100px;margin:auto;padding:48px 24px}h1{font-size:clamp(36px,6vw,64px);line-height:1.1;margin:14px 0}
h2{margin-bottom:4px}.eyebrow{color:#78e5ca;letter-spacing:.16em;font-size:12px;font-weight:700}
p{color:#b4bfd4}.cards{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:24px;margin:32px 0}
section{background:#1a2130;border:1px solid #344157;border-radius:16px;padding:22px;min-width:0}
.scroll{overflow-x:auto}table{width:100%;border-collapse:collapse;font-size:14px}td,th{text-align:left;padding:12px 8px;border-bottom:1px solid #344157}
th{color:#b4bfd4;font-size:12px}td:first-child{color:#78e5ca}.tempo{height:6px;background:#78e5ca;border-radius:4px;margin-top:5px}
.stats{font-size:24px;color:#fff}.note{border-left:3px solid #78e5ca;padding:0 16px}.warn{color:#ffc288}
@media(max-width:850px){.cards{grid-template-columns:1fr}main{padding:30px 16px}}
</style><main><div class="eyebrow">DJLAB / MEMORY MEETS MUSIC</div><h1>Set Journey</h1>)HTML";
    out << "<p>" << escape_html(playlist_name) << " · " << orders[0].size()
        << " requests · " << config.controller_cache_size << " cache slots · tolerance "
        << config.bpm_tolerance << " BPM</p><div class=cards>";
    DJLibraryService library;
    library.buildLibrary(config.library_tracks);
    for (int mode = 0; mode < 2; ++mode) {
        // Each route starts with an empty cache. No warm-cache advantage.
        DJControllerService controller(config.controller_cache_size);
        library.loadPlaylistFromIndices(playlist_name, orders[mode]);
        size_t hits = 0, misses = 0, evictions = 0;
        out << "<section><div class=eyebrow>" << (mode == 0 ? "YOUR ORDER" : "TEMPO SUGGESTION")
            << "</div><h2>" << (mode == 0 ? "Original route" : "Nearest BPM route")
            << "</h2><p class=stats>" << total_jump(config, orders[mode]) << " total BPM jump</p>"
            << "<div class=scroll><table><thead><tr><th># / Track</th><th>BPM</th><th>Jump</th><th>Cache</th></tr></thead><tbody>";
        for (size_t i = 0; i < orders[mode].size(); ++i) {
            const auto& track = get_track(config, orders[mode][i]);
            AudioTrack* loaded = library.findTrack(track.title);
            if (!loaded) throw std::runtime_error("Missing playlist track");
            int status = controller.loadTrackToCache(*loaded);
            if (status == -2) throw std::runtime_error("Track clone failed");
            if (status == 1) ++hits;
            else ++misses;
            if (status == -1) ++evictions;
            int jump = i == 0 ? 0 : std::abs(track.bpm - get_track(config, orders[mode][i-1]).bpm);
            out << "<tr><td>" << i+1 << ". " << escape_html(track.title) << "</td><td>" << track.bpm
                << "<div class=tempo style=\"width:" << (track.bpm * 100 / 1000) << "px\"></div></td>"
                << "<td" << (jump > config.bpm_tolerance ? " class=warn" : "") << ">"
                << (i == 0 ? "—" : std::to_string(jump)) << "</td><td>"
                << (status == 1 ? "HIT" : status == -1 ? "MISS / EVICT" : "MISS") << "</td></tr>";
        }
        out << "</tbody></table></div><p>" << hits << " hits · " << misses << " misses · "
            << evictions << " evictions</p></section>";
    }
    out << R"HTML(</div><p class=note>The suggestion preserves the opening track and every request, including repeats.
It greedily chooses the nearest remaining BPM, keeps input order on ties and falls back to the original route if the total jump grows.
This is a tempo heuristic, not a globally optimal or musically complete set recommendation.</p>
<p>Cache events come from the actual C++ LRU controller, starting empty for each route. BPM values are original metadata before auto-sync.
No audio is decoded or played. Lower tempo movement does not guarantee better cache performance or a better live set.</p></main></html>)HTML";
    out.close();
    if (!out) throw std::runtime_error("Report write failed");
}
