# Delivery verification / 交付校验

Verified on 2026-10-15 (Asia/Shanghai), Apple M1 Pro, macOS 27.2, arm64.
M4 Pro and older macOS versions are not physically verified in this session.

- The one-command build script prepared project-local pinned tools and built libFLAC
  from the verified 1.5.0 source archive with a macOS 13 deployment target.
- Native bridge, Bonjour discovery, local FLAC/PCM and bounded RAOP probe compiled.
  Runtime dependencies reported by the native bridge/probe are system libraries and
  system frameworks; OpenSSL, mbedTLS and codecs require no Homebrew runtime install.
- Complete current suite: 48/48 PASS; project address/UB instrumentation: 48/48 PASS.
- Clang static analysis: six new audio/policy/discovery/probe translation units,
  zero diagnostics. Shell syntax and git whitespace checks passed.
- Packaged executables passed ad-hoc strict signature checks, fresh ZIP extraction
  and SHA-256 file manifest verification.
- The extracted installer passed in a separate path containing spaces; state was
  0700 and config 0600. Installed local FLAC reported a 24-bit original source and
  exactly 44,100 output frames. LOSSLESS failed explicitly with exit 2. Invalid
  modes failed. One-click uninstall moved only that test installation to Trash.
- Actual user installation and LaunchAgent bootstrap passed. The background job
  entered running state, used the observed LAN interface, discovered HomePod through
  system Bonjour, created an ALAC renderer and registered the Connect service.
- Repeated one-click AUTO startup recognized the existing owned PID without a
  duplicate process or advertisement. No global networking settings were changed.
- Actual HomePod accepted the RAOP session; local source submission passed, while
  receiver audibility and actual Spotify playback/controls remain UNVERIFIED.
- cspot hosted workflows at 16180c8c7999c3d80b6ae967d089b3822f96d927 succeeded:
  macOS local audio (38617040515), upstream clang-format 14 (38617040329), and
  upstream Ubuntu CLI build (38617040285). Their jobs actually executed.

The main bridge hosted workflow result is reported in the release and final delivery.
Package ZIP hashes are in the accompanying .sha256 file; embedding a ZIP's own hash
inside itself would be self-referential. SOURCE-MANIFEST.json identifies exact source
and submodule commits. This is a Preview, not a Spotify Lossless-certified release.

Preview.2 packaging-time follow-up: the Mac was unlocked and the real Spotify
picker inspected. That personal hotspot had no observed HomePod service. Four additional
process-local Bonjour lifecycle tests pass in both builds. Receiver reconnection
was UNVERIFIED at packaging; the later real LAN follow-up is recorded below and
in LIVE-VALIDATION.md.

- Preview.2 fresh extraction: file hashes and four strict signatures PASS.
- Isolated installation in a path containing spaces, local 24-bit FLAC decoding,
  explicit LOSSLESS refusal, private permissions and recoverable uninstall PASS.
- Real upgrade and live reinstallation PASS; LaunchAgent/native process running
  on the hotspot at that time. Repeated AUTO startup kept the PID. VORBIS_320/AUTO
  switches kept agent management and graceful shutdown. No physical music test
  is inferred from those mode changes.
- Missing-interface exit 75 and LOSSLESS exit 2 retained the running AUTO service.
- Updated native Bonjour static analysis reported zero diagnostics. Shell syntax
  and whitespace checks passed.
- Explicit interface handoff to the managed process PASS; the private request was
  consumed once and later AUTO startup used automatic selection. Unsupported
  loopback selection failed before stopping the existing service.
- Native recovery code at f6e3beafaeca2154c60ce03748e15df204de25cf passed the executed
  [hosted ARM64 workflow](https://github.com/hhhoratioxu/SpotConnect/actions/runs/38658723934),
  including 48 native / 48 instrumented tests, package and artifact upload. Later
  launcher verification is covered by the local lifecycle checks above; final
  release workflow evidence is recorded in its release notes.
- Package privacy regression PASS: an ignored private file with a license-like
  name and unique marker was excluded. The package retained 118 Git-source and
  libFLAC license files, with no private test extraction paths among its inputs.

## Public release and real LAN follow-up

- Exact released source 872af596a1394dbfa94339b375e604526506eaf8 passed
  [hosted run 38660172218](https://github.com/hhhoratioxu/SpotConnect/actions/runs/38660172218):
  48 native / 48 project-instrumented tests, packaging and artifact upload executed.
- Preview.2 was downloaded again from the public GitHub release. ZIP SHA-256:
  `99cb32d5bdbcb2ebdd243f0304b41b4a08f892f7c5f9fade4fb9b2ba5b1a1504`.
  Fresh extraction verified 140 file hashes, four strict ad-hoc signatures, the
  exact released commit and cspot 16180c8c7999c3d80b6ae967d089b3822f96d927.
  The published tag and assets were not replaced for this documentation update.
- The user returned the Mac to the HomePod LAN. The existing managed installation
  recovered HomePod discovery, renderer creation and Connect service publication.
  A fresh ZeroConf getInfo returned status 101 / OK. This is an actual recovery
  observation, separate from process-local tests and extended stability testing.
- After the user manually selected the HoriSpot device, the real account session,
  metadata and audio-key response succeeded. AUTO selected OGG_VORBIS_320 in
  metadata. The user reported no music. Empty application credentials prevented
  token/CDN URL requests; no Spotify audio bytes reached a decoder. Music playback
  was BLOCKED at that attempt, not a successful Vorbis or FLAC result or server denial.
- The real HomePod again accepted ALAC, 44.1kHz / 16-bit / stereo. A new original
  local 24-bit FLAC submission supplied 44,100 attenuated frames. The user did not
  notice the quiet test, so audibility remains UNVERIFIED. The test is local audio,
  not Spotify source or end-to-end bit-perfect evidence.

## Preview.3 diagnostics and later account evidence

- Local native and project-instrumented suites: 52/52 PASS each. Four added tests
  encode independent signed PCM with the actual ALAC/ALAC_RAW encoders linked by
  RAOP and compare every sample through Apple's reference decoder, including
  stereo extrema and short-packet zero padding. Dependencies remain precompiled.
- PCM submission now reports frame counts, nonzero samples, peak and session/sane
  state. A down session or rejected chunk retains pending PCM. Session connection
  no longer depends on an enabled INFO logging macro. Physical music verification
  of these diagnostics is separate from compilation/offline checks.
- Full changed-unit Clang analysis: PCM C++ unit had zero diagnostics; C main had
  19 diagnostic messages, identical to analysis of the pre-change source. These
  existing findings are not presented as a completely clean whole-bridge audit.
- After explicit owner approval, a new developer application was created and its
  credentials configured privately. Both direct token preflight and the bridge
  fetched a token. Actual Spotify CDN resolution/download and Vorbis 320 decoder
  opening passed, reporting 44100 Hz / stereo / 16-bit PCM. The user still reported
  no music; audible output is FAIL, while controls/stability remain unaccepted.
- A known-values privacy scan found no application credentials in 74,283 tracked
  source files, runtime logs or the previously published Preview.2 archive.
- Two fresh local 3-second ALAC/ALAC_RAW probes each supplied 132,300 attenuated
  frames, received receiver timing exchanges and had no observed audio send errors.
  The user did not notice them; no audible pass is recorded.


- Exact Preview.3 code a692eeaaf75532dc3d33b00e367090909488a00b passed
  [hosted run 38683473524](https://github.com/hhhoratioxu/SpotConnect/actions/runs/38683473524),
  including 52 native / 52 project-instrumented tests, packaging and artifact upload.
- Preview.3 local ZIP SHA-256:
  `48288531a9f3055ad20e2ac04616187d2dbaff1f7edf5cfc76fa7b6f8dc1a48e`.
  Fresh extraction verified 141 file hashes, four strict ad-hoc signatures and the
  exact code/core manifest. Isolated installation in a path containing spaces,
  local decode, LOSSLESS refusal and recoverable uninstall passed. The real
  installation upgraded while preserving its private application credentials.
- The latest user-selected Preview.3 music attempt reached 1,241,856 submitted PCM
  frames over 28 reports, with nonzero samples and streaming/sane state. Fourteen
  timing replies and no observed audio socket errors are sender/network evidence.
  The user explicitly reported no sound. Receiver music acceptance remains FAIL;
  no bit-perfect, stability or Spotify Lossless success follows from these stats.
- The verified Preview.3 ZIP was separately scanned for the actual private app
  credentials: zero matches. Values remain only in the protected local config.
