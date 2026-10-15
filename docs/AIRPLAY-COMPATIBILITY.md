# AirPlay compatibility investigation / AirPlay 兼容性排查

2026-10-15, Asia/Shanghai. Experimental. No audible pass or Spotify FLAC claim.

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
It currently remains a separate local diagnostic, not a bundled HoriSpot backend.

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
