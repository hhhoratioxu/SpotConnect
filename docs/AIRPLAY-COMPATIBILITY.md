# AirPlay compatibility investigation / AirPlay 兼容性排查

Updated 2026-10-20, Asia/Taipei. Experimental. Local tone and real music are separate.

The actual HomePod's AirPlay TXT advertises model AudioAccessory6,1 and osvers 27.2.
Legacy RAOP accepted ALAC / 44100 Hz / 16-bit stereo. Real Spotify Vorbis decoding
passed after private application configuration, but the user reported no music.
Local ALAC/ALAC_RAW probes each supplied 132,300 attenuated source frames and
received five timing responses with no observed audio socket send errors. The
user did not notice those tones, so local receiver audibility remains UNVERIFIED.

Four additional offline tests use the same prebuilt ALAC encoders linked by RAOP
and Apple's reference decoder. Full/short blocks, signed extrema, different
stereo channels and raw-packet zero padding match independent source PCM exactly.
This rules out sample corruption in those tested codec paths, not every possible
streaming, network or receiver problem.

## Additional upstream evidence

[music-assistant/airplay-cli PR 74](https://github.com/music-assistant/airplay-cli/pull/74)
was merged on 2026-09-19. Its author reports a native AirPlay 2 clock-role fix for
some standalone HomePods on OS 27. That is a candidate compatibility explanation;
it is not proof of this HomePod's failure or a fix implemented in SpotConnect's
legacy RAOP transport.

The official [airplay-cli v0.5.8 release](https://github.com/music-assistant/airplay-cli/releases/tag/v0.5.8)
was inspected separately. Tagged source commit:
`1701bc25e9d303b42532569f624b64c0d0afb1f6` (annotated tag object
`52cc4392e1f03e5265f0a8d90203263ca404bc98`). GPL-3.0 and bundled third-party
notices were read; no code, license notices or dependency history were replaced.
The official macOS arm64 asset passed its published SHA256SUMS:
`6d460ad08562844d978f337bee28a79b0eea390c84c239b5d66f55624e27d719`.
At that investigation stage it remained a separate local diagnostic. The
2026-10-20 candidate below bundles the unchanged pinned sender.

A bounded native AirPlay 2 / NTP test with original local s16le PCM established
the encrypted session, reported audio buffered, acknowledged START, and disconnected
with exit 0. Requested input was 44100 Hz, 16-bit stereo at 20% device volume.
That handshake/start test does not establish audible output or a receiver-side
transport specification beyond the tested session.

PTP mode uses UDP 319/320. Direct socket binds as the current user returned EACCES
for both on this Mac. No administrator authorization was inferred from ordinary
project permissions. No root daemon, privileged port workaround, network setting,
HomePod access-control change or persistent system service was installed.

当前已经验证 Spotify 真实 Vorbis 数据可以下载并进入解码器，HomePod 音乐仍被用户
确认无声。OS 27 播放时钟修复提供了新的排查方向；目前没有把这个候选原因说成
已确认根因，也没有把另一个发送库的握手说成可听播放通过。PTP 管理员权限和真实
可听输出仍须独立验证。Spotify FLAC 访问仍未实现。


The installed Preview.3 real-song retest produced 1,241,856 nonzero-containing
PCM frames over 28 reports while the sender stayed in streaming/sane state.
Fourteen receiver timing replies and no observed audio socket errors accompanied
this run. The user again reported silence. The compatibility hypothesis remains
unconfirmed: sender-side counters cannot establish a receiver render timeline.


## 2026-10-20 native AirPlay 2 candidate

A fresh Bonjour query found the same AudioAccessory6,1 receiver with OS 27.2,
using different DHCP addresses. The old PTP comparison had no execution artifacts.
Its hard-coded addresses were unsuitable after a network change; that administrator
launcher has been retired locally without executing it.

Using the newly resolved peer, port and local discovery interface, a bounded
native AirPlay 2 / NTP session sent six seconds of original interleaved s16le PCM.
The user explicitly confirmed hearing the alternating tones. The sender reported
native/NTP routing, receiver format capabilities, SETUP, volume acceptance and
START. This is an actual local-audio audible PASS, not a Spotify playback pass.
The diagnostic's final FIFO cleanup initially failed because the child had already
removed its FIFO; that cleanup was corrected. The audible confirmation and raw
status evidence are retained independently from that process-cleanup failure.

The candidate routes the tested HomePod 2 model through the unchanged official
airplay-cli v0.5.8 sender, pinned by SHA-256 and corresponding-source submodule.
Other models keep their existing RAOP route. HORISPOT_AIRPLAY_OUTPUT=legacy selects
the old route explicitly. The native route requests 44100 Hz / 16-bit stereo,
uses NTP without privilege, starts at 20% volume, bounds pending PCM, waits for
flush acknowledgments and supervises child exit. Five packaged executables retain
signatures and source/license references. Native metadata/artwork propagation and
HomePod button feedback have not been implemented for this candidate.

Seven synthetic child-process tests verify exact PCM pipe bytes, control
commands and warm flush, child failure, bounded pre-start buffering, and selection
of only the actual peer's advertised properties. Full native and project ASan/UB
suites pass 59/59; changed adapter static analysis reports no findings.

Initial installed integrated music attempts were still silent. Explicit native
connect probes succeeded on the OS route's Ethernet interface but timed out on
Wi-Fi. The first launcher had selected the first Bonjour result, which was not
necessarily that route. After a temporary Ethernet selection, one real-song
attempt was paused by a controller command before its planned audible start.
A later locally requested resume submitted about ten seconds of cumulative PCM
and then stalled; receiver audibility did not pass.

The subsequent fixes prefer the OS route, refresh actual discovery TXT even with
an explicit bind, and forward only peer-matched advertised fields to the sender.
The CLI's DURATION/PROGRESS contract uses seconds; the adapter's millisecond input
is now converted and tested. Sender media-consumption status is reported separately
from PCM submission. With these changes the same integrated adapter submitted all
264,600 frames of a six-second original tone and reported consumption advancing to
about 5.4 seconds. This establishes that tested local pipeline's forward progress;
its acoustic confirmation and the corrected real-music result remain pending.
The earlier direct-tone audible PASS is not substituted for those missing results.
