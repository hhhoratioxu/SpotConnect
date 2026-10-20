# Upstream audit / 上游审计

Checked live on 2026-10-15, Asia/Shanghai, through GitHub API and complete core Git
histories. Source changes are distinguished from account/server evidence.

Refreshed the two core repositories live on 2026-10-20 (Asia/Taipei). SpotConnect's
latest default-branch commit is `a533cc39ea54a47a10504f0bfb0c92e30c34a4c1`, dated
2026-10-15, adding CONTRIBUTING.md; the bridge is still based on the 0.20.11 code
below. cspot remains at `99e5c95ad6d862a1b7845dda81fd9c8233b45b9f`. This audit does
not turn contributor reports into this project's physical acceptance results.

[SpotConnect PR #84](https://github.com/philippe44/SpotConnect/pull/84) and its
[libraop PR #55](https://github.com/philippe44/libraop/pull/55) remain open and
propose native AirPlay 2/NTP for OS 27 receivers. The PR describes rebuilt static
archive requirements and unimplemented receiver controls. It has not been merged
or adopted wholesale in this fork; the Preview.4 candidate instead supervises the
pinned Music Assistant sender. [Issue #89](https://github.com/philippe44/SpotConnect/issues/89)
records a separate intermittent sender handover report. None of these changes adds
a Spotify FLAC provider or proves this HomePod's music output.

| Repository | Audited source and role | License |
| --- | --- | --- |
| philippe44/SpotConnect | 227884c68c6d2706281c630e4e041e02a04d1afa, 0.20.11, 2026-10-03 | MIT |
| philippe44/cspot | 99e5c95ad6d862a1b7845dda81fd9c8233b45b9f, 2026-10-09 | LICENSE.md states GPL-3.0-or-later; GitHub automatic SPDX detection is NOASSERTION |
| vin-w/hass-spotconnect-addons | main, last pushed 2026-06-30; Home Assistant packaging | No root license observed; not copied/forked because HA is not needed |
| GioF71/spotconnect-docker | main, last pushed 2026-10-04; Docker packaging | MIT; not the audio decoder |

Fork parents were verified as philippe44/SpotConnect and philippe44/cspot. Development
uses only feature/spotify-lossless. No PR was submitted to either upstream.

## Recent updates

SpotConnect includes fixes through 0.20.11; newer UPnP PRs include #81 (merged) and
#82 (open). cspot #5, which fixes declaration of all capabilities, was merged on
2026-10-07 and is included. cspot #4 is open and concerns stale Notify messages,
not FLAC. No FLAC implementation was found in the current audited cores or PRs.

[SpotConnect #64](https://github.com/philippe44/SpotConnect/issues/64#issuecomment-3798257485)
contains the maintainer's statement that lossless implementation is not planned.
[HomePod #71](https://github.com/philippe44/SpotConnect/issues/71) is an open no-audio
report, so reliability cannot be assumed from discovery.
[librespot #1583](https://github.com/librespot-org/librespot/issues/1583#issuecomment-3499343020)
records Spotify's communication about technical protections and the maintainers'
decision to stop development circumventing them. This fork does not implement
PlayPlay, extract official client keys/binaries, or bypass account or server controls.

## Audio and authorization

TrackQueue fetches Mercury metadata, chooses a file, requests a legacy audio key,
and resolves a signed CDN URL with an application token. CDNAudioFile uses the
upstream AES-CTR path and a 167-byte Spotify Vorbis prefix. TrackPlayer uses
ov_open_callbacks / ov_read and provides signed 16-bit PCM bytes. These assumptions
are Vorbis-specific, not a verified FLAC pipeline.

The original cspot schema ended at AAC_48. The public
[librespot schema](https://github.com/librespot-org/librespot/blob/dev/protocol/proto/metadata.proto)
defines FLAC_FLAC=16 and FLAC_FLAC_24BIT=22. Recognition was added; AAC_24=8 remains
AAC. Neither recognition nor a local decoder authorizes a Spotify file or audio key.

Spotify documents eligible Premium lossless audio up to 24-bit/44.1kHz FLAC on
supported devices in its [official announcement](https://newsroom.spotify.com/2025-09-10/lossless-listening-arrives-on-spotify-premium-with-a-richer-more-detailed-listening-experience/).
That does not grant this receiver access. The
[Commercial Hardware SDK program](https://developer.spotify.com/documentation/commercial-hardware)
is an official integration route requiring authorization and suitable SDK capability.
The [public Web API](https://developer.spotify.com/documentation/web-api) does not
document an API returning full-track FLAC audio to this bridge. Application
credentials alone must not be labeled FLAC authorization.

Account-specific FLAC metadata, keys, eligibility and quality selection were not
verified in this session. This client has no authorized FLAC provider. AUTO reports
the limitation; LOSSLESS fails. There is no lossless capability or invented success log.

## HomePod and output

Native Bonjour and the bridge found the actual HomePod 2. Observed RAOP TXT advertised
codecs 0,1,2,3, encryption types 0,3,5 and model AudioAccessory6,1, with no sr/ss/ch.
Missing values were not promoted to hardware specifications. A subsequent actual
44.1kHz/16-bit/stereo AppleLossless/ALAC RAOP session was accepted by that receiver.
This is a session result, not a general HomePod or AirPlay 2 specification.

[Apple HomePod lossless instructions](https://support.apple.com/en-lamr/guide/homepod/apd210c0eb0a/homepod)
describe native Apple Music playback. They do not prove this RAOP path's format or
bit-perfect behavior. No 24-bit/high-rate AirPlay output or internal DAC behavior is claimed.

Source URLs and dependency refs ship in SOURCE-MANIFEST.json. The bridge uses pinned
upstream native static libraries; independent libFLAC is built from checksum-verified
1.5.0 source. Underlying codec/TLS licenses are retained separately from the MIT
packaging-wrapper licenses.
