# Delivery verification / 交付校验

Verified on 2026-10-15 (Asia/Shanghai), Apple M1 Pro, macOS 27.2, arm64.
M4 Pro and older macOS versions are not physically verified in this session.

- The one-command build script prepared project-local pinned tools and built libFLAC
  from the verified 1.5.0 source archive with a macOS 13 deployment target.
- Native bridge, Bonjour discovery, local FLAC/PCM and bounded RAOP probe compiled.
  Runtime dependencies reported by the native bridge/probe are system libraries and
  system frameworks; OpenSSL, mbedTLS and codecs require no Homebrew runtime install.
- Complete current suite: 44/44 PASS; project address/UB instrumentation: 44/44 PASS.
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
