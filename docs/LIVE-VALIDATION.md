# Unlocked live validation / 解锁后的真实环境检查

2026-10-15, Asia/Shanghai. Preview / Experimental.

## Hotspot inspection before release packaging

The later LAN follow-up below supersedes the account/reconnection gates in this
historical hotspot section. It does not establish successful Spotify playback.

The Mac was unlocked by the user. The existing official Spotify session was
available and its real Connect picker was inspected. It listed the phone as the
current player and this computer as an available device. The HomePod bridge was
absent. The phone displayed a Lossless badge; no bridge FLAC bytes, metadata or
decoder evidence was obtained from that badge.

The user confirmed that the Mac was using a personal hotspot. Read-only system
interface checks showed that the previous HomePod LAN address was no longer
assigned to the Mac. Native Bonjour discovery returned zero matching HomePod
services. An unfiltered scan returned the Mac's own AirPlay advertisements. The
previous HomePod peer did not respond to a bounded reachability check. These
observations supported a missing common LAN, not a Spotify audio success or failure.
System Wi-Fi, hotspot, proxy, DNS and routes were not changed.

No HomePod target could be selected. Bridge account login, Spotify-provided
Vorbis/FLAC, receiver audibility, pause/seek/next/volume and sustained playback
were **UNVERIFIED** at that point. The current client deliberately leaves Spotify FLAC access
disabled and LOSSLESS refuses playback. Private application credentials were not
supplied or extracted from the official application.

This inspection also exposed a stale-bind problem in Preview.1: the process was
alive while still bound to its former network address. Preview.2 checks the bind
inside native Bonjour, releases discovery and receiver resources when it becomes
invalid, and exits with temporary-failure status 75. The user LaunchAgent retries
failed exits with a 30-second throttle and selects the current discovery interface
or active Wi-Fi address. Absence of a HomePod means waiting for discovery; it does
not create a fake receiver. Explicit mode changes retain the registered agent.

Four tests redirect interface enumeration **only in the test executable** and
exercise the real scoped Bonjour query: missing bind, bind lost after browsing,
explicit stop, and bounded completion. The 48 native and 48 project-instrumented
checks pass. These simulations do not verify physical network switching or actual
HomePod reconnection; the later real observation is recorded separately below.

The extracted Preview.2 package passed its file hashes and all four strict ad-hoc
signature checks. Installation into a separate path containing spaces passed;
state permissions were 0700 and config permissions 0600. Its original local
24-bit FLAC fixture decoded to 44,100 PCM frames. LOSSLESS refused with exit 2.
Uninstall moved that test installation to recoverable Trash.

The user's existing installation was upgraded and the real LaunchAgent entered
running state on the hotspot at that time. Repeated AUTO startup retained the same
native PID. Explicit VORBIS_320 and AUTO changes each produced a new native PID
managed by the same agent, after graceful cleanup. LOSSLESS and a nonexistent
interface refused with exits 2 and 75 respectively without stopping the managed
AUTO process. A second live installation also passed. No HomePod Connect target
was advertised on the hotspot. These are lifecycle checks, not music playback.

The explicit current LAN interface was also passed through a managed mode change;
the next native process used it and consumed the private one-time request. A later
AUTO start resumed automatic selection. A loopback override was rejected before
stopping the existing service, because the upstream RAOP bridge requires a usable
LAN address. These checks did not change any system interface.

用户解锁后已实际打开 Spotify Connect 列表；热点环境下没有 HoriSpot HomePod 目标。
iPhone 的 Lossless 标签不能证明桥接客户端取得 FLAC。未进行桥接账号登录和歌曲
播放，因此不会把播放、Seek、切歌、音量或可听输出记为通过。此前在 HomePod LAN
上已取得的发现、RAOP 握手及本地 FLAC PCM 发送证据，仍保留其各自的测试范围。

## Return to the HomePod LAN and real account session

Later on the same date, the user returned the Mac to the HomePod LAN. Without
changing system networking, the existing user-managed Preview.2 recovered the
native HomePod service, created the renderer and published its Connect service.
The listener used the current physical LAN address; a new ZeroConf getInfo request
returned 101 / OK. This verifies this physical return-to-LAN recovery case, not
every possible network transition or continuous playback stability.

The user manually selected the HoriSpot target in official Spotify and reported
silence. At approximately 12:06 local time the bridge recorded:

1. `SPOTIFY_CLIENT_CREDENTIALS_REQUIRED`, explicitly saying no token request was sent.
2. Actual track metadata selection: `AUTO`, `OGG_VORBIS_320`, fallback reason
   `SPOTIFY_FLAC_ACCESS_UNAVAILABLE`.
3. A successful audio-key response after the authenticated Connect session.
   Key values, track identifiers and account information are not published.
4. Track-load timeout. No CDN URL and no decoder `AUDIO_SOURCE` result followed.

The installed application's Client ID and Client Secret were checked only for
presence; both were empty. AccessKeyFetcher therefore did not issue a token
request, and TrackQueue did not send the storage-resolve request. This is a local
configuration blocker, not evidence of server rejection. The account session and
metadata/key steps PASS; actual Spotify audio download, decode and music output
are BLOCKED. The Premium/FLAC entitlement and real controls are still UNVERIFIED.

A new bounded probe on the same LAN received HomePod session acceptance for
AppleLossless / ALAC at 44.1kHz, 16-bit, stereo. A second probe decoded the original
24-bit local fixture and submitted 44,100 attenuated PCM frames. The user was not
sure whether the quiet tone was heard. Sender submission PASS does not establish
audibility, Spotify audio, or end-to-end bit-perfect output.

The official developer dashboard was opened with the user's manual sign-in. It
showed no existing applications. No application secret was copied from the
official Spotify player, another application, or upstream binaries.

用户返回局域网后，HomePod 发现和 Connect 广播已经恢复；用户选中了目标设备，
真实账号会话、曲目元数据及音频密钥取得成功。当前无声卡在音乐下载前：个人应用
凭据为空，因此没有发送 token/CDN 地址请求，也没有 Spotify 音频字节或解码器源
格式证据。补齐自己的获准应用凭据后，还要实际验证服务器是否允许下载，再验收
可听音乐、暂停、Seek、切歌、音量和稳定性。真实 Spotify Lossless 仍未实现。

## Owner-approved application and actual Spotify audio

The owner explicitly approved creating HoriSpotConnect in their developer account
and accepting Spotify's developer terms/design guidelines. The app was created
through the official dashboard. Its visible credentials were transferred directly
to the installed private XML, mode 0600; a 0600 temporary transfer file was removed.
Values were never printed or committed. A credential preflight received HTTP 200
and a token with 3600 seconds lifetime, without logging or saving that token.

The owned user LaunchAgent was restarted. The user selected HoriSpot again and
reported no sound. This attempt, at approximately 12:25 local time, produced:

- Bridge token issuance success, metadata selection of OGG_VORBIS_320, audio-key
  responses, signed CDN URL resolution and CDN stream opening.
- Actual decoder `AUDIO_SOURCE`: `codec=OGG_VORBIS_320 sample_rate=44100 channels=2
  source_bit_depth=not_applicable decoded_pcm_bits=16 lossless=0`.
- Accepted RAOP LOAD and PLAY sessions, streaming-state entry, progress setup and
  volume commands. These are sender-side events, not audible-output proof.
- A later Mercury read error/reconnection event. Extended stability does not pass.

Thus the initial configuration blocker was resolved and lawful account/app Vorbis
download/decoding passed on this installation. HomePod music output still FAILS
the user's audible check. No Spotify FLAC bytes were requested or decoded. No
conclusion about another account, application or FLAC entitlement follows.

A privacy scan of 74,283 tracked source files, runtime logs and the downloaded
public Preview.2 package found no matches for the new application credentials.

Both local encoder paths were then tested with an original 3-second 24-bit FLAC,
attenuated before sending: ALAC and ALAC_RAW each supplied 132,300 frames and
completed accepted HomePod sessions. Each log contained five timing responses,
learned the receiver timing port, and recorded no audio socket send errors. The
user did not notice the tones, so these remain sender/timing evidence only.

AirPlay TXT advertises `osvers=27.2`. A separately downloaded, checksum-verified
official airplay-cli v0.5.8 connected through native AirPlay 2 with NTP timing,
buffered local PCM and acknowledged START, then disconnected with exit 0. It was
not integrated into the bridge and does not prove audible music. PTP clock mode
requires UDP 319/320; direct unprivileged binds returned EACCES. System networking
and device access settings were not changed. See AIRPLAY-COMPATIBILITY.md.


## Installed Preview.3 and real-song PCM

The user selected the installed Preview.3 target and played again, then reported
silence. The actual Spotify source remained OGG_VORBIS_320 / 44100 Hz / stereo /
16-bit decoded PCM. Across 28 reporting intervals, the sender accepted 1,241,856
frames (about 28.16 seconds). The PCM contained nonzero samples; session state
was streaming and sender sanity remained true. The receiver answered 14 timing
exchanges. No audio socket send error was observed in the diagnostic interval.
A later Mercury read error/reset occurred, so sustained playback is unaccepted.

These facts narrow the problem beyond initial account/token/download setup.
They do not prove UDP receipt, receiver decoding, valid render timing or audible
sound. The user-facing music check remains **FAIL**, including this Preview.3
attempt. No Spotify FLAC data was requested or decoded.

Exact Preview.3 source a692eeaaf75532dc3d33b00e367090909488a00b passed the executed
[hosted ARM64 workflow](https://github.com/hhhoratioxu/SpotConnect/actions/runs/38683473524):
52 native tests, 52 project-instrumented tests, packaging and artifact upload.
Fresh local extraction verified 141 file hashes and four strict ad-hoc signatures.
Isolated installation, original 24-bit FLAC decode, explicit LOSSLESS refusal and
recoverable uninstall passed. A known-values scan found no application credentials
in the Preview.3 package.
