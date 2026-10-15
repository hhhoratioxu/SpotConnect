# Unlocked live validation / 解锁后的真实环境检查

2026-10-15, Asia/Shanghai. Preview / Experimental.

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
observations support a missing common LAN, not a Spotify audio success or failure.
System Wi-Fi, hotspot, proxy, DNS and routes were not changed.

No HomePod target could be selected. Bridge account login, Spotify-provided
Vorbis/FLAC, receiver audibility, pause/seek/next/volume and sustained playback
remain **UNVERIFIED**. The current client deliberately leaves Spotify FLAC access
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
HomePod reconnection; that acceptance gate remains **UNVERIFIED**.

The extracted Preview.2 package passed its file hashes and all four strict ad-hoc
signature checks. Installation into a separate path containing spaces passed;
state permissions were 0700 and config permissions 0600. Its original local
24-bit FLAC fixture decoded to 44,100 PCM frames. LOSSLESS refused with exit 2.
Uninstall moved that test installation to recoverable Trash.

The user's existing installation was upgraded and the real LaunchAgent entered
running state on the current hotspot. Repeated AUTO startup retained the same
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

下一次真实验收需要 Mac 与 HomePod 在同一可发现局域网、合法的应用凭据（如上游
路径要求），并依次取得实际元数据、解码器、发送会话和可听结果。应用凭据本身
不保证 Spotify FLAC 授权；真实 Lossless 仍需要被允许的源访问接口。
