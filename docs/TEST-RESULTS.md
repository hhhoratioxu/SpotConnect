# Test results / 测试结果

2026-10-15, Asia/Shanghai. Preview / Experimental. No Spotify FLAC success claimed.

| Check | Executed result | Scope / limit |
| --- | --- | --- |
| Core/fork/source audit | PASS | Live GitHub API and local Git; exact hashes in UPSTREAM-AUDIT.md |
| ARM64 native compilation | PASS | AppleClang 21 / C++20, bridge and diagnostics |
| Complete offline suite | 48/48 PASS | 28 FLAC/policy/PCM + 16 cspot metadata/CDN/AES/Vorbis + 4 Bonjour lifecycle tests |
| Address + UB instrumentation | 48/48 PASS | Project source instrumented; precompiled dependency archives not instrumented |
| Shell and package checks | PASS | Syntax, isolated/live installation, re-extraction, hashes, signatures |
| LaunchAgent / mode lifecycle | PASS | Idempotent AUTO, VORBIS_320/AUTO switches, refusal gates and live reinstall; no music playback implied |
| HomePod 2 discovery | PASS | Actual LAN receiver; raw name/IP kept outside Git |
| Bridge renderer creation | PASS | Actual HomePod renderer and ALAC encoder created |
| Connect advertisement | PASS | Actual _spotify-connect._tcp service observed |
| ZeroConf HTTP getInfo | PASS | Returned status 101 / OK and actual receiver name |
| HomePod RAOP session | PASS | Accepted 44.1kHz/16-bit/stereo AppleLossless session |
| Local FLAC → PCM → RAOP | PASS for sender submission | Original 24-bit fixture, 44,100 frames; low amplitude test |
| Actual audible output | UNVERIFIED | No manual/acoustic receiver confirmation |
| Spotify UI device picker | CHECKED; target selection BLOCKED | After unlock, phone/computer available; target HomePod absent on hotspot |
| Bind disappearance / query shutdown | 4/4 PASS | Process-local simulated interface loss and real scoped Bonjour query; no system network changes |
| Premium / current FLAC permission | UNVERIFIED | Account-specific authorized FLAC source not tested |
| Real Spotify Vorbis playback | UNVERIFIED | Personal application credentials and UI pairing not completed |
| Real Spotify FLAC source/decode | UNVERIFIED / unavailable in this client | Local fixtures are not Spotify evidence |
| Real pause / seek / next / volume | UNVERIFIED | Offline seek passed; real controller sequence outstanding |
| Continuous stream stability | UNVERIFIED | Short local submission is not extended playback evidence |
| HomePod end-to-end bit-perfect | UNVERIFIED | Source encoding differs from conversion and receiver processing |
| Developer ID / notarization | UNVERIFIED | Ad-hoc signatures only |
| GitHub hosted workflow | Recorded at delivery | Workflow code or a queued job is not an executed build |

Assertions are active in optimized builds. Fixtures originate as original PCM,
are encoded with libFLAC's verification mode, and are compared sample for sample
against that original PCM. A new EOF-seek test caught a callback lifetime issue;
it was repaired before final verification. Final suite passes.

Vorbis fixtures have verified nominal 96/160/320 headers and are decoded through
actual CDNAudioFile HTTP Range/AES-CTR using a loopback-only server. Header/footer,
small file, last byte/EOF, random seeks, truncation and oversized-body guards are tested.
These are synthetic transport tests, not Spotify service tests.

AudioToolbox tests measure duration, pitch, amplitude and channel separation for
48/96kHz conversion. 16-bit 44.1kHz PCM is compared exactly; 24-bit reduction checks
saturation and one-LSB bounds. Fixtures are original CC0-1.0 audio.

Raw live logs are in local ignored private-test-results/. Public reports contain
technical outcomes only. Packaged JUnit XML records offline names and timings.

The unlocked UI follow-up is recorded in [LIVE-VALIDATION.md](LIVE-VALIDATION.md).
It does not upgrade account playback, source-format or HomePod audibility gates.
