# HoriSpotConnect changelog

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
