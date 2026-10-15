# Real environment acceptance / 真实环境验收

This is an executable Preview, not an accepted Spotify Lossless product. Each gate
below needs separate evidence before a stable release.

1. Connect the Mac to the HomePod LAN. Use official Spotify's Connect picker, select the receiver ending
   in [HoriSpot], and verify the intended Premium account.
2. Configure the user's own permitted application Client ID/Secret in installed
   0600 state/config.xml if required by the upstream token path. Do not use another
   application's credentials or put secrets in Git/issues.
3. Test actual Vorbis 320 first; separately capture selected metadata encoding,
   decoder format, RAOP session acceptance and receiver audibility.
4. Verify pause/resume, seeking including zero, next/previous, volume feedback,
   disconnect/reconnect and sustained playback; measure interruptions and latency.
5. Spotify FLAC requires an authorized receiver/provider integration. Subscription,
   enum recognition and UI labels are insufficient. Verify source format and
   decoder STREAMINFO against actual lawfully provided bytes. Do not redistribute
   commercial audio, tokens or keys.
6. Only after that provider exists can AUTO genuinely prefer FLAC and LOSSLESS play.
   Verify unavailable/denied/missing FLAC fallback to available 320 in AUTO, and
   explicit refusal in LOSSLESS. Unknown formats/access remain fail-closed.
7. Report source FLAC separately from 44.1kHz/16-bit stereo RAOP output. Do not claim
   24-bit/high-rate/bit-perfect HomePod output without receiver-side evidence.

hori-discover and hori-audio-probe need no Spotify credentials. The bounded
hori-raop-probe defaults to opening/closing a session while preserving device
volume. Optional local FLAC sends at most three seconds at reduced amplitude with
an eight-second deadline. It labels local source and audible output unverified.

已执行：返回 HomePod 局域网后恢复发现和 Connect 注册、设备信息请求、实际
RAOP 握手、原创本地 FLAC 的 44,100 帧发送。用户手动选中目标后，真实 Connect
账号会话、曲目元数据和音频密钥取得成功，元数据选择 Vorbis 320。个人应用凭据
为空，下载地址请求尚未发出；用户报告歌曲无声，因此 Spotify 音乐播放为 BLOCKED。
本地测试音未获听到确认，可听输出、真实控制和稳定性保留 UNVERIFIED。正式版本
必须等待相应验收完成。详见 [LIVE-VALIDATION.md](LIVE-VALIDATION.md)。
