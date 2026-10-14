# Real environment acceptance / 真实环境验收

This is an executable Preview, not an accepted Spotify Lossless product. Each gate
below needs separate evidence before a stable release.

1. Unlock the Mac, use official Spotify's Connect picker, select the receiver ending
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

已执行：HomePod 发现、Connect 注册、设备信息请求、实际 RAOP 握手、原创本地 FLAC
的 44,100 帧发送。账号、界面、可听输出、控制和稳定性保留 UNVERIFIED。正式版本
必须等待相应验收完成。
