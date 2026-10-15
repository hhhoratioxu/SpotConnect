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

Preview.2 follow-up: the Mac was unlocked and the real Spotify picker inspected.
The current personal hotspot has no observed HomePod service. Four additional
process-local Bonjour lifecycle tests pass in both builds. Receiver reconnection
after a physical network change remains UNVERIFIED; see LIVE-VALIDATION.md.

- Preview.2 fresh extraction: file hashes and four strict signatures PASS.
- Isolated installation in a path containing spaces, local 24-bit FLAC decoding,
  explicit LOSSLESS refusal, private permissions and recoverable uninstall PASS.
- Real upgrade and live reinstallation PASS; LaunchAgent/native process running
  on the current hotspot. Repeated AUTO startup kept the PID. VORBIS_320/AUTO
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
