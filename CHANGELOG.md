# HoriSpotConnect changelog

## Verification follow-up — 2026-10-15

Documentation only; published Preview.2 binaries/tag remain unchanged.

- Verify the real return from hotspot to the HomePod LAN: managed discovery,
  renderer creation, Connect advertisement and ZeroConf getInfo recovered.
- Record user target selection and silence; actual Connect session, metadata and
  audio-key response passed, with OGG_VORBIS_320 selected in metadata.
- Identify empty personal application credentials as the blocker before token/
  CDN URL requests. Do not claim server rejection or received Spotify audio.
- Repeat accepted HomePod ALAC session and 44,100-frame local FLAC submission;
  receiver audibility remains UNVERIFIED because the user did not notice the tone.
- Re-download public Preview.2; verify ZIP SHA-256, 140 file hashes, four strict
  signatures and exact source references. Final hosted source run passed 48/48
  native and 48/48 project-instrumented tests.

## 0.1.0-preview.2 — 2026-10-15

Preview / Experimental. Spotify FLAC access remains unavailable.

- Inspect the unlocked official Spotify device picker; record the absent HomePod
  target on the current personal hotspot without treating the phone's Lossless
  badge as bridge evidence.
- Detect a disappeared native Bonjour bind address and cleanly end the bridge;
  let the user LaunchAgent retry discovery with its existing 30-second throttle.
- Validate the current interface/address before idempotent startup and retain
  LaunchAgent management on explicit mode changes.
- Restrict PID-based termination to this installation's executable paths.
- Collect dependency licenses from Git-tracked source; exclude private test
  extractions and ignored runtime files from package inputs.
- Add four isolated Bonjour lifecycle tests; execute 48 native and 48
  project-instrumented checks. Physical HomePod reconnection was UNVERIFIED at
  packaging; the later real follow-up is recorded above.

## 0.1.0-preview.1 — 2026-10-15

Preview / Experimental. Genuine Spotify FLAC access remains unavailable.

- Preserve SpotConnect 0.20.11 / latest cspot history, copyright and licenses.
- Strict AUTO, LOSSLESS and VORBIS_320; preserve legacy Vorbis 96/160/320.
- Recognize public FLAC IDs without declaring an unsupported Connect capability.
- Add local libFLAC decoding, checksum/truncation checks, bounded PCM, seek including
  after EOF, and AudioToolbox format conversion.
- Fix final CDN reads, small-file footer arithmetic, signed seek bounds, oversized
  responses, failed Vorbis opening, sink format checks and seek-to-zero handling.
- Redact CDN signed URLs and pairing secrets; reject missing private application
  credentials and contain token/network exceptions.
- Fix the documented -o model filter, previously matching the service name.
- Add native Bonjour, LAN interface selection and explicit source/output/session logs.
- Add user installation, startup, recoverable uninstall and login LaunchAgent.
- Add ARM64 build/package scripts, exact source manifest, hashes and signature checks.
- Execute 44 offline / 44 sanitizer tests; verify actual HomePod discovery, RAOP
  handshake and 44,100 submitted frames from an original local 24-bit FLAC.
- Retain unverified Spotify FLAC, audible output, real controls, extended streaming,
  end-to-end bit-perfect, Developer ID and notarization gates.

[Original upstream release history](CHANGELOG) is retained.
