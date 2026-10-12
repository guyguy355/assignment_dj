#include "MP3Track.h"
#include "WAVTrack.h"
#include "Playlist.h"
#include "LRUCache.h"
#include "DJControllerService.h"
#include "MixingEngineService.h"
#include "DJSession.h"
#include "SetJourney.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <functional>
#include <algorithm>
#include <type_traits>
#include <cstdio>

void check(bool condition) {
    if (!condition) throw std::runtime_error("Check failed");
}
void throws(const std::function<void()>& operation) {
    bool caught = false;
    try { operation(); } catch (const std::exception&) { caught = true; }
    check(caught);
}
PointerWrapper<AudioTrack> track(const std::string& name, int bpm = 120) {
    return PointerWrapper<AudioTrack>(new MP3Track(name, {"Test"}, 180, bpm, 320));
}
struct ProbeTrack : MP3Track {
    static int live;
    bool fail_clone;
    bool fail_load;
    ProbeTrack(const std::string& name, bool fail = false)
        : MP3Track(name, {"Test"}, 180, 120, 320), fail_clone(fail), fail_load(false) { ++live; }
    ProbeTrack(const ProbeTrack& other)
        : MP3Track(other), fail_clone(other.fail_clone), fail_load(other.fail_load) { ++live; }
    ProbeTrack& operator=(const ProbeTrack& other) = default;
    ~ProbeTrack() { --live; }
    PointerWrapper<AudioTrack> clone() const override {
        if (fail_clone) throw std::runtime_error("Injected clone failure");
        return PointerWrapper<AudioTrack>(new ProbeTrack(*this));
    }
    void load() override {
        if (fail_load) throw std::runtime_error("Injected load failure");
    }
    void set_sample(double value) { waveform_data[0] = value; }
};
int ProbeTrack::live = 0;

std::string config_text() {
    return "library_track_2=WAV,B,{Artist;},200,140,44100,16\n"
           "library_track_1=MP3,A,{Artist;},180,120,320,1\n"
           "controller_cache_size=2\nnight=1,2,1\n";
}
bool parse(const std::string& text, SessionConfig& config) {
    std::ofstream out("build/test-config.txt"); out << text; out.close();
    return SessionFileParser::parse_config_file("build/test-config.txt", config);
}

int main() {
    int passed = 0, failed = 0;
    auto test = [&](const std::string& name, const std::function<void()>& run) {
        try { run(); ++passed; std::cout << "PASS " << name << std::endl; }
        catch (const std::exception& e) { ++failed; std::cerr << "FAIL " << name << ": " << e.what() << std::endl; }
    };
    test("pointer move, release, reset and null access", [] {
        PointerWrapper<int> a(new int(7));
        PointerWrapper<int> b(std::move(a));
        check(!a && *b == 7);
        b.reset(b.get()); check(*b == 7);
        int* raw = b.release(); check(!b); delete raw;
        throws([&] { (void)*b; });
        throws([&] { (void)b.operator->(); });
        PointerWrapper<int>* same = &b;
        b = std::move(*same); check(!b);
    });
    test("audio waveform deep copy and assignment", [] {
        ProbeTrack a("A"); a.set_sample(0.5);
        ProbeTrack b(a); a.set_sample(0.2);
        double sample = 0; b.get_waveform_copy(&sample, 1); check(sample == 0.5);
        b = a; a.set_sample(0.8); b.get_waveform_copy(&sample, 1); check(sample == 0.2);
    });
    test("audio move and self assignment", [] {
        MP3Track a("A", {"Test"}, 180, 120, 320);
        MP3Track b(std::move(a)); check(b.get_title() == "A");
        a = std::move(b); check(a.get_title() == "A");
        a = a; MP3Track* same = &a; a = std::move(*same); check(a.get_title() == "A");
    });
    test("playlist preserves insertion order and owns clones", [] {
        Playlist a("A"); a.add_track(track("one").release()); a.add_track(track("two").release());
        check(a.getTracks()[0]->get_title() == "one");
        Playlist b(a); b.find_track("one")->set_bpm(150);
        check(a.find_track("one")->get_bpm() == 120);
        a.remove_track("one"); check(b.get_track_count() == 2);
    });
    test("playlist move empties source", [] {
        Playlist a; a.add_track(track("A").release());
        AudioTrack* original = a.find_track("A");
        Playlist b(std::move(a)); check(a.is_empty() && b.find_track("A") == original);
        a = std::move(b); check(b.is_empty() && a.find_track("A") == original);
    });
    test("failed playlist copy frees partial clones", [] {
        Playlist a; a.add_track(new ProbeTrack("A")); a.add_track(new ProbeTrack("B", true));
        int before = ProbeTrack::live;
        throws([&] { Playlist b(a); }); check(ProbeTrack::live == before);
        Playlist target; target.add_track(track("safe").release());
        throws([&] { target = a; }); check(target.find_track("safe") != nullptr);
    });
    test("zero capacity rejected", [] {
        throws([] { LRUCache cache(0); });
        LRUCache cache(2); cache.put(track("A"));
        throws([&] { cache.set_capacity(0); }); check(cache.contains("A"));
    });
    test("true LRU eviction after hit", [] {
        LRUCache cache(2); cache.put(track("A")); cache.put(track("B")); cache.get("A");
        check(cache.put(track("C"))); check(cache.contains("A") && !cache.contains("B"));
    });
    test("cache shrink keeps most recent and grows safely", [] {
        LRUCache cache(3); cache.put(track("A")); cache.put(track("B")); cache.put(track("C")); cache.get("A");
        cache.set_capacity(1); check(cache.size() == 1 && cache.contains("A"));
        cache.set_capacity(4); cache.put(track("D")); check(cache.size() == 2);
    });
    test("duplicate cache entry keeps existing track", [] {
        LRUCache cache(2); cache.put(track("A", 120)); cache.put(track("A", 130));
        check(cache.size() == 1 && cache.get("A")->get_bpm() == 120);
    });
    test("controller reports hits without cloning", [] {
        ProbeTrack a("A"); DJControllerService controller(2);
        check(controller.loadTrackToCache(a) == 0); a.fail_clone = true;
        check(controller.loadTrackToCache(a) == 1);
    });
    test("controller reports eviction", [] {
        DJControllerService controller(1); auto a = track("A"), b = track("B");
        check(controller.loadTrackToCache(*a) == 0);
        check(controller.loadTrackToCache(*b) == -1);
    });
    test("failed load does not leak clone", [] {
        ProbeTrack a("A"); a.fail_load = true; DJControllerService controller;
        int before = ProbeTrack::live;
        throws([&] { controller.loadTrackToCache(a); }); check(ProbeTrack::live == before);
    });
    test("deck clone survives cache eviction", [] {
        MixingEngineService mixer; DJControllerService controller(1);
        auto a = track("A"), b = track("B"); controller.loadTrackToCache(*a);
        check(mixer.loadTrackToDeck(*controller.getTrackFromCache("A")) >= 0);
        controller.loadTrackToCache(*b); check(mixer.can_mix_tracks(a));
    });
    test("BPM sync reaches active BPM without changing source", [] {
        MixingEngineService mixer; auto a = track("A", 120), b = track("B", 160);
        mixer.loadTrackToDeck(*a); auto copy = b->clone(); mixer.sync_bpm(copy);
        check(copy->get_bpm() == 120 && b->get_bpm() == 160);
        throws([&] { mixer.set_bpm_tolerance(-1); });
    });
    test("parser orders numeric IDs and repeated reads replace data", [] {
        SessionConfig config; check(parse(config_text(), config)); check(config.library_tracks[0].title == "A");
        check(parse(config_text(), config)); check(config.library_tracks.size() == 2);
    });
    test("parser rejects invalid config without modifying output", [] {
        SessionConfig config; check(parse(config_text(), config));
        std::vector<std::string> bad = {
            "controller_cache_size=0\n", "controller_cache_size=-2\n", "controller_cache_size=2junk\n",
            "auto_sync=maybe\n", "bpm_tolerance=-1\n", "night=1,99\n", "night=1,bad\n", "night=1,\n",
            "library_track_1=MP3,A,{X;},-1,120,320,1\n", "library_track_1=MP3,A,{X;},180,0,320,1\n",
            "library_track_3=MP3,C,{X;},180,120,320,1\n", "broken line\n"
        };
        for (const auto& text : bad) {
            check(!parse(text, config)); check(config.library_tracks.size() == 2);
        }
    });
    test("parser rejects duplicate titles and invalid fields", [] {
        SessionConfig config;
        check(!parse(config_text()+"library_track_3=MP3,A,{X;},180,120,320,1\n", config));
        check(!SessionFileParser::validate_track_format("MP3,A,{X;},180,120junk,320,1"));
        check(!SessionFileParser::validate_track_format("WAV,A,{X;},180,120,44100,7"));
    });
    test("journey preserves opener, duplicates and improves known route", [] {
        SessionConfig config; check(SessionFileParser::parse_config_file("examples/night_drive.txt", config));
        auto original = config.playlists["night-drive"];
        auto suggested = SetJourney::suggest_order(config, original);
        check(suggested.front() == original.front());
        check(SetJourney::total_jump(config, suggested) < SetJourney::total_jump(config, original));
        std::sort(original.begin(), original.end()); std::sort(suggested.begin(), suggested.end()); check(original == suggested);
        check(SetJourney::suggest_order(config, {}).empty());
        throws([&] { SetJourney::suggest_order(config, {99}); });
    });
    test("journey HTML escapes user metadata", [] {
        SessionConfig config; check(parse(config_text(), config));
        config.library_tracks[0].title = "<script>&\"";
        SetJourney::write_report(config, "night", "build/test-report.html");
        std::ifstream file("build/test-report.html");
        std::string html((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        check(html.find("<script>") == std::string::npos);
        check(html.find("&lt;script&gt;&amp;&quot;") != std::string::npos);
        throws([&] { SetJourney::write_report(config, "missing", "build/missing.html"); });
    });
    test("multiple demo sessions process every playlist", [] {
        std::ostringstream output;
        std::streambuf* original = std::cout.rdbuf(output.rdbuf());
        try {
            { DJSession a("First", true); a.simulate_dj_performance(); }
            { DJSession b("Second", true); b.simulate_dj_performance(); }
        } catch (...) { std::cout.rdbuf(original); throw; }
        std::cout.rdbuf(original);
        std::string result = output.str();
        size_t second = result.find("Session: Second");
        check(second != std::string::npos);
        check(result.find("Tracks processed: 10", second) != std::string::npos);
        check(result.find("Cache hits: 7", second) != std::string::npos);
        check(result.find("Transitions: 9", second) != std::string::npos);
    });
    check(ProbeTrack::live == 0);
    std::cout << "RESULT: " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
