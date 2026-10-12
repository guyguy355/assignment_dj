# Changes from the uploaded project

The original C++11 service layout, naming conventions, explicit loops, linked playlist, polymorphic track hierarchy and custom pointer wrapper were retained. Changes are focused on behavior and explainability rather than replacing the project with a new framework.

## Correctness fixes

- Controller HIT returns immediately instead of falling through to cloning and reporting a MISS. Preparation keeps a clone owned during exceptions.
- Playlist insertion preserves input order. Deep-copy construction cleans partial lists on failure; assignment uses copy-and-swap. Move operations transfer nodes rather than triggering another deep copy.
- Audio copy assignment prepares allocations and metadata before replacing old resources. Waveform allocation follows random-generator initialization, avoiding an unowned allocation if initialization throws.
- Cache rejects zero capacity. Shrinking evicts by recency and compacts surviving entries rather than truncating arbitrary vector positions. Empty slots do not claim ownership of a null track.
- Pointer dereference and arrow access reject null. `get()` still returns nullptr normally; release and move semantics are preserved.
- Library service copying is disabled to prevent shallow copies of owning raw pointers. Newly constructed tracks remain owned if vector insertion throws.
- Mixer BPM sync now reaches the active deck's BPM instead of only averaging toward it. Cache/source metadata remains unchanged.
- Session playlist cursor is local rather than static across instances. Transition counts exclude the initial deck load. Failed controller loads skip mixer transfer. Missing configuration/playlists fail the CLI.
- Config parsing validates numeric fields, IDs, references and unique title keys, orders numbered tracks correctly and commits only a complete valid result. Invalid entries are rejected instead of silently shifting track indices.

## Added

- `SetJourney`: deterministic nearest-BPM route with stable tie handling and non-worsening fallback; original-versus-suggested cache replay and escaped HTML output.
- A fictional eight-request demo, a generated report and a plain CLI (`journey`, `demo`, `interactive`).
- Executable regression tests, sanitizer target, dependency-aware Makefile and Hebrew setup/interview notes.

## Removed or clarified

- Exercise TODOs, instructor prompts, phase-based console demos and obsolete grading targets.
- The unused second `ConfigurationManager` parser and unused declarations for an unimplemented playlist-file API. `SessionFileParser` is the active config path.
- Contradictory borrowing comments: playlist nodes actually own their tracks.
- Legacy phase CLI flags. Commands and simulation limitations are documented in README.

No Git history was rewritten and nothing was published to GitHub. Original collaborative/course provenance is retained in NOTICE. No claim is made that this simulation decodes or mixes real audio.
