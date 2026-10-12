# DJLab — Set Journey

A C++11 DJ session simulator focused on ownership, polymorphic copies and a bounded LRU cache. **Set Journey** compares the original playlist with a simple tempo-based suggestion and shows the actual cache events for both routes.

This project simulates track preparation and deck transitions. It does not decode MP3/WAV files, play audio, detect beats from sound or perform real audio mixing. Waveforms are synthetic and quality scores are illustrative formulas.

## Quick start

Use Linux, macOS with a C++ compiler and Make, or Ubuntu in WSL on Windows. Commands run from the `DJLab` directory:

```bash
make
make demo
```

Open `report.html` in your browser. The included `demo-preview/set-journey.html` can also be opened directly without compiling.

```bash
./bin/dj_manager journey examples/night_drive.txt night-drive report.html
./bin/dj_manager demo
./bin/dj_manager interactive
make test
make sanitize
```

`demo` and `interactive` use `bin/dj_config.txt`. The `journey` command accepts a config path, playlist name and output HTML path. The output directory must already exist. Usage errors return 2; configuration/runtime errors return 1. Old exercise CLI flags have been replaced by these commands.

## What the report demonstrates

The example contains eight track requests and a two-slot cache. Both routes start with an empty controller cache.

| Metric | Original route | Suggested route |
| --- | ---: | ---: |
| Sum of absolute adjacent BPM changes | 98 | 22 |
| Cache hits | 0 | 3 |
| Cache misses | 8 | 5 |
| Evictions | 6 | 3 |

The suggestion keeps the opening track, then repeatedly chooses the nearest remaining BPM. Ties preserve input order; repeated requests remain present. If its total BPM jump exceeds the original route, the original is retained. Complexity is O(n²) time and O(n) auxiliary space. It does not claim global optimality and does not consider musical key, mood or artist variety. Consecutive repeats can improve these metrics without making a better live set.

The report uses original BPM metadata, before mixer auto-sync. The mixer changes only its own clone when synchronizing. The report replays requests through `DJControllerService`; its cache numbers are not a separately simulated approximation.

## Ownership and structure

| Component | Owns | Borrowed pointers |
| --- | --- | --- |
| `DJLibraryService` | Library tracks and current playlist | `findTrack()` returns a playlist track |
| `Playlist` | Linked-list nodes and each track | `getTracks()` and `find_track()` do not transfer ownership |
| `LRUCache` | Slots containing `PointerWrapper<AudioTrack>` | `get()` is valid until eviction, clear, or destruction |
| `MixingEngineService` | Independent polymorphic track clones on decks | Input track is only borrowed during loading |
| `PointerWrapper` | One object, never an array | `get()` borrows; `release()` transfers ownership |

`AudioTrack` implements the Rule of Five for its waveform buffer. Playlist copies clone every track; moves transfer nodes. `PointerWrapper` is move-only. The small cache uses vector scans: lookup, insertion and eviction selection are O(capacity), not O(1). Playlist append scans the linked list; building a playlist is O(n²). These choices keep the original structure visible and are suitable for the small included examples.

## Configuration

See `examples/night_drive.txt`. Library IDs must be contiguous from 1, but lines may appear out of order. Titles must be unique because the existing cache uses titles as keys. Playlist indices may repeat. Parsing is transactional: rejected input leaves the previous configuration unchanged.

Supported ranges: cache capacity 1–1024, BPM 1–1000, duration 1–86400 seconds, MP3 bitrate 1–320, tags 0/1, WAV sample rate 1–384000 and bit depth 8/16/24/32. These are simulator bounds, not an audio-format validation specification. Commas inside fields and quoted CSV are not supported. `auto_sync` accepts true/false or 1/0.

## Verification and limits

See [validation](docs/VALIDATION.md) for actual commands and results, including the LeakSanitizer environment limitation. The suite checks ownership, deep copies, injected failures, LRU order, config rejection, deck isolation and report escaping.

The simulator is single-threaded. No real-time audio guarantees, concurrent cache access or persistent storage are provided. `PointerWrapper` is retained as an educational implementation; a production application would normally use `std::unique_ptr`. Tests cover selected failures, not every possible allocation failure. No Windows-native executable is included; compile under WSL.

## Walkthrough and history

- [Start here — Hebrew](START_HERE_HE.md)
- [Changes and rationale](docs/CHANGES.md)
- [Origin and attribution](NOTICE.md)

The original classes and C++11 style remain: headers/sources, explicit loops, the linked playlist, Rule of Five and pointer wrapper. The additions were developed with AI assistance and should be reviewed and understood before presenting them as your own work.
