# Validation record

Date: 2026-10-11. Environment: Linux, g++ 13.3.0, C++11.

| Check | Result |
| --- | --- |
| `make` | Passed |
| `make test` | 21 named scenarios passed, 0 failed |
| `make sanitize ASAN_OPTIONS=detect_leaks=0` | Same 21 scenarios passed; no ASan or UBSan errors reported |
| `make sanitize` with leak detection enabled | Blocked at LeakSanitizer shutdown: environment denied access to process/task data |
| Original config, `./bin/dj_manager demo` | 10 processed, 7 hits, 3 misses, 0 evictions, 9 transitions, 0 errors |
| Set Journey example | HTML generated; original 98 BPM total / 0 hits / 6 evictions, suggested 22 BPM total / 3 hits / 3 evictions |
| Unknown CLI command | Exit 2 |
| Missing journey config | Exit 1 |
| Demo launched without its config in working directory | Exit 1 |

The original uploaded program was also built and run before editing. It reported 10 misses and 0 hits for the same original configuration; the corrected result is 3 misses and 7 hits.

## What the suite covers

Move/release/reset/null pointer behavior; independent waveform copies; audio moves; playlist order, deep copies and moves; injected clone failure with cleanup and unchanged assignment target; zero cache capacity; LRU touch/eviction; resizing; duplicate insertion; hit without cloning; eviction status; injected preparation failure; deck clone lifetime after eviction; BPM sync isolation; transactional parsing and repeated reads; malformed config and field rejection; route opener and multiset preservation; HTML escaping; and independent repeated DJ sessions.

## Limits

LeakSanitizer did not complete in this environment: it reported inability to inspect `/proc/.../task` and a fatal environment error. Therefore this delivery does **not** claim a completed leak scan. Targeted live-object checks passed, but do not replace a general leak detector. Default `make sanitize` keeps leak detection enabled for a compatible local environment.

Windows/WSL setup instructions were not executed on a Windows host. The HTML report was generated and checked for its contents/escaping; a browser screenshot could not be obtained because the browser binary was unavailable. There is no real audio playback or audio-quality validation. No GitHub upload was performed.
