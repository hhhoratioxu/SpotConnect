# HoriSpotConnect — macOS ARM64 Preview

**真实 Spotify FLAC Lossless 尚未实现。 / Genuine Spotify FLAC access is not implemented.**

An inspectable fork of SpotConnect 0.20.11 and cspot, with a native macOS bridge,
strict quality policies, tested local libFLAC decoding, and separate real-device evidence.
Original history, upstream licenses and copyright notices are retained.
[Original SpotConnect README](README.upstream.md) · [cspot fork](https://github.com/hhhoratioxu/cspot/tree/feature/spotify-lossless)

## 中文

这是 Preview / Experimental，不是通过 Spotify Lossless 验收的正式版本。无需
Home Assistant。不能把 Vorbis 转 ALAC、Spotify UI 的 Lossless 标签，或本地 FLAC
测试当作 Spotify 无损源成功的证据。

已完成 macOS ARM64 可执行桥接、独立 libFLAC 解码和 PCM 转换、严格音质模式、
HomePod 发现、启动/安装/卸载脚本与用户级 LaunchAgent。当前会话完成了 HomePod 2
RAOP 握手及原创本地 FLAC 的 PCM 发送；从热点回到局域网后，服务恢复发现，用户
选中了 Connect 设备。经用户确认创建自己的开发者应用、私密配置后，token、真实
曲目元数据、音频密钥、CDN 下载和 Vorbis 解码均已通过。用户仍报告歌曲无声，
因此 HomePod 音乐输出验收为 FAIL；真实控制与连续播放仍未验收。

2026-10-20 分支上的 Preview.4 候选版加入原生 AirPlay 2 / NTP 发送端。本地原创
6 秒交替测试音已得到用户明确的可听确认，无需管理员时钟进程。首次完整适配层
的歌曲复测仍无声；随后修正了有线/Wi-Fi 路由选择、实际接收端 TXT 传递和进度
单位，完整适配层的测试音媒体进度已推进到约 5.4 秒。修正后的歌曲发声仍待验收，
不能把该本地测试结果当作 Spotify 音乐或 Lossless 成功。

### 音质与支持矩阵

| 功能 | 当前实际状态 |
| --- | --- |
| AUTO | 明确报告 Spotify FLAC 不可用，选择元数据中可用的 Vorbis 320；缺少 320 则失败 |
| LOSSLESS | 报错 SPOTIFY_FLAC_ACCESS_UNAVAILABLE，退出码 2；不播放伪无损 |
| VORBIS_320 | 固定选择 Vorbis 320，缺少 320 则失败 |
| 原有 Vorbis 96/160/320 | 保留；未设置质量环境变量时沿用 upstream 的 -r 行为 |
| 本地 FLAC | libFLAC 1.5.0；已测 16/24/32 位、mono/stereo、Seek、损坏/截断处理 |
| Spotify FLAC 元数据 | 识别公开格式 ID 16/22；识别不等于能获取或播放 |
| Spotify FLAC 源选择/访问 | 此客户端 UNAVAILABLE；没有 DRM/权限绕过或官方客户端密钥提取 |
| HomePod 2 发现与网络恢复 | 返回共同局域网后恢复发现和 renderer 创建，实测 PASS |
| Connect 服务与设备信息 | 注册与 ZeroConf 接口 PASS；用户已手动选中目标设备 |
| Spotify Connect 账号会话 | PASS；取得真实曲目元数据和音频密钥，元数据选中 Vorbis 320 |
| Spotify Vorbis 下载与解码 | PASS；自己的应用 token、CDN 地址和实际解码器：Vorbis 320 / 44.1 kHz / stereo / 16-bit PCM |
| RAOP 输出 | 返回局域网后再次实测 HomePod 接受 ALAC 会话，44.1 kHz / 16-bit / stereo |
| Preview.4 原生 AirPlay 2 / NTP | 仅对已测 HomePod 2 型号启用；独立本地测试音已确认可听；完整适配层与歌曲需分别验收 |
| 本地 FLAC → PCM → RAOP | 实际提交 44,100 帧；低幅度测试，可听输出 UNVERIFIED |
| Spotify → HomePod 音乐 | FAIL for audible output；取流与解码通过，用户再次报告无声，继续定位发送/接收兼容性 |
| 暂停 / Seek / 切歌 / 音量 | 控制路径保留；离线 Seek 测试 PASS，真实 Spotify 控制 UNVERIFIED |
| 长时间连续播放 | UNVERIFIED |
| HomePod 端到端 bit-perfect | UNVERIFIED；24→16 位和重采样会改变样本 |
| 签名 | 本地 ad-hoc 签名验证；无 Developer ID 公证 |

### 安装与运行

下载 [Preview 发布包](https://github.com/hhhoratioxu/SpotConnect/releases/tag/v0.1.0-preview.3)，
解压并运行 scripts/install.sh。只写入当前用户目录，不需要 sudo。

```sh
curl -fL https://github.com/hhhoratioxu/SpotConnect/releases/download/v0.1.0-preview.3/HoriSpotConnect-0.1.0-preview.3-macos-arm64.zip -o HoriSpotConnect.zip
ditto -xk HoriSpotConnect.zip .
bash HoriSpotConnect-0.1.0-preview.3-macos-arm64/scripts/install.sh
```

安装器验证 SHA-256 和签名，安装到 ~/Library/Application Support/HoriSpotConnect，
默认启用登录自启。--no-launchagent 可只安装。双击 start.command 一键启动，
双击 uninstall.command 一键卸载；卸载将应用和私有状态移入废纸篓以便恢复。

```sh
"$HOME/Library/Application Support/HoriSpotConnect/start.command" AUTO
"$HOME/Library/Application Support/HoriSpotConnect/start.command" VORBIS_320
"$HOME/Library/Application Support/HoriSpotConnect/start.command" LOSSLESS
```

默认模式保存在 state/quality.mode。日志在 logs/stderr.log 与 logs/stdout.log。
实际源日志来自选中的元数据和已打开的解码器，包含编码、采样率、声道和 PCM
位深；Vorbis 无整数源位深，报告 source_bit_depth=not_applicable。
AIRPLAY_OUTPUT 给出发送契约与请求编码，libraop 报告实际 ALAC/PCM 编码状态。
握手接受与可听输出分别记录。

启动器发现 HomePod 后优先采用系统到该接收端的路由接口，避免多网卡 Bonjour
结果顺序导致误选 Wi-Fi；同时刷新实际接收端能力信息供原生发送端使用。
只绑定本程序；可用 HORISPOT_INTERFACE=en7 等覆盖，未修改系统代理或网络配置。
Preview.2 检测绑定地址失效后清理连接，登录启动项以 30 秒节流重新发现并绑定。
手动启动检查当前接口与地址，切换音质仍交由已有启动项管理。热点上没有 HomePod
时只等待发现，不宣称已连接或播放。
显式接口选择会传给下一次受管理的启动；后续网络失效会重新自动选择。

### Spotify 登录与真实限制

在官方 Spotify 应用中选择名字后带 [HoriSpot] 的设备，以 upstream ZeroConf
方式登录。没有复制官方应用的令牌、密码或私钥。

上游自 2025-12 起在自编译版本中要求自己的 application Client ID 和 Client
Secret 获取其 Web API token（[上游说明](https://github.com/philippe44/SpotConnect/blob/master/README.md)）。
这组凭据标识开发者应用，Spotify 会员账号通过 Connect 登录是另一个环节。将自己的信息只填入安装目录 state/config.xml
的 `<client_id>` / `<client_secret>`，该文件权限为 0600。这些应用凭据不是 Premium
账号的 Lossless 授权，也不能解锁 FLAC。缺少凭据时清晰报错，不发送占位符或
打印秘密。

当前真实测试已完成 Connect 账号会话、应用 token、元数据/音频密钥、CDN 下载和
Vorbis 解码。第一次无声确实缺少应用凭据；配置后仍无声，不能把这个旧阻塞继续
当作当前原因，也不能把解码成功当作 HomePod 发声。应用可在
[Spotify 官方后台](https://developer.spotify.com/dashboard) 查看；新建
应用涉及开发者条款和凭据创建，需由账号所有者确认。2026 年新的 Development Mode
还有 Premium、单个 Client ID 和接口范围限制，详见
[官方更新](https://developer.spotify.com/blog/2026-02-06-update-on-developer-access-and-platform-security)。
本机应用已实际取得 Vorbis 下载权限；这不保证其他应用、账号或 FLAC 访问。
根节点下需要实际的 `<client_id>` / `<client_secret>` 元素，注释里的示例不会生效。

Preview.3 增加实际 PCM 提交帧数、非零样本数、16 位峰值和发送端状态日志，并在
会话关闭或数据块被拒绝时保留待发送 PCM。连接调用独立于日志级别。新增四项
ALAC 编码/参考解码测试验证完整与短数据块逐样本还原；这些检查不代替真实发声。
HomePod 广播 `osvers=27.2`，另一个上游已修复部分 OS 27 的播放时钟兼容问题；
目前只是候选原因，详见 [AirPlay 兼容性记录](docs/AIRPLAY-COMPATIBILITY.md)。

已核查最新源码/Issues/PR，见 [上游审计](docs/UPSTREAM-AUDIT.md)。SpotConnect
维护者尚无实现 Lossless 的计划；librespot 维护者报告 Spotify 技术保护限制。
官方 Commercial Hardware SDK/批准的集成需要进一步取得授权与接口能力。当前
cspot 的 Mercury 元数据、legacy key 请求与 Vorbis 专用 CDN 适配器，不能被
当作已经验证的 Spotify FLAC 访问方式。

### 构建与测试

需要已安装的 Apple Command Line Tools、Python 3 和 Git。脚本在项目内部准备
固定版本工具和 libFLAC，不安装 Homebrew、不修改 SDK 许可或全局 Git 配置。

```sh
git clone -b feature/spotify-lossless https://github.com/hhhoratioxu/SpotConnect.git HoriSpotConnect
cd HoriSpotConnect
bash scripts/build-macos.sh
bash scripts/build-macos.sh --sanitize
bash scripts/package-macos.sh
```

Git 中保留源码和 submodule 精确引用，产物附 SOURCE-MANIFEST.json 与许可证。
当前分支完整套件 **59/59 PASS**，地址/未定义行为检查 **59/59 PASS**；覆盖原始
PCM 逐样本比较、CDN/AES/HTTP Range 路径、Vorbis 96/160/320 解码与 Seek，
以及 4 项隔离的 Bonjour 接口生命周期、4 项 ALAC 编码/解码测试、7 项原生发送
适配层检查（精确 PCM、控制、失败、限流及接收端能力匹配）；不会修改系统网络。
音频为原创合成、CC0-1.0，未分发 Spotify 商业录音。
[完整测试记录](docs/TEST-RESULTS.md) · [真实环境验收](docs/ACCEPTANCE.md)

构建目标最低 macOS 13；本机结果不代表已测试所有 macOS 版本或 M1/M4 机型。
GitHub Actions 仅构建公开源码和离线测试，不连接个人 Spotify/HomePod。

## English

This is a **Preview / Experimental** macOS Apple Silicon fork. Genuine Spotify
FLAC streaming is **not implemented**. The Spotify path remains Vorbis. Encoding
that PCM as ALAC avoids another lossy encode, but cannot restore discarded data.

AUTO reports unavailable Spotify FLAC access and selects available Vorbis 320.
LOSSLESS fails explicitly, without playback or a false capability claim.
VORBIS_320 requires that format. Generic callers retain legacy 96/160/320 selection.

The separate local path uses libFLAC 1.5.0 and macOS AudioToolbox, preserves signed
16/24/32-bit decoded samples, validates corruption/truncation, and supports seek
including restarting after EOF. It converts to 44.1kHz/16-bit/stereo RAOP PCM.
Mono duplication, bit-depth reduction and resampling are reported separately.

On the tested HomePod 2, discovery and RAOP session acceptance succeeded. An
original local 24-bit FLAC supplied 44,100 PCM frames to the real ALAC/RAOP sender,
with deliberate attenuation for the short test. Receiver audibility and real
controls remain **UNVERIFIED**. After the user returned from a personal hotspot
to the HomePod LAN, the managed bridge recovered discovery and its Connect
advertisement. The user selected the target and reported silence. The real
Connect account session, track metadata and audio-key response succeeded. The
initial attempt stopped before download because application credentials were
empty. The owner then approved creating an application and accepting its terms;
credentials were stored privately. Token issuance, signed CDN resolution and
actual Vorbis 320 decoding subsequently passed (44.1kHz, stereo, 16-bit PCM).
The user again reported silence: end-to-end music output is **FAIL**, not an
accepted playback result. Application credentials do not grant FLAC rights or
guarantee access for another app/account under Development Mode restrictions.

The 2026-10-20 Preview.4 candidate adds the pinned official native AirPlay 2/NTP
sender for the tested HomePod 2 model. The user explicitly heard the independent
six-second original tone; this required no privileged clock process. The first
integrated real-music attempts still failed the audible check. Subsequent fixes
select the OS route to the receiver, forward only its actual discovered TXT, and
convert progress milliseconds to the sender's seconds contract. The corrected
integrated local-tone test submitted 264,600 frames and advanced sender media
consumption to about 5.4 seconds; its acoustic confirmation and corrected real
music remain separate acceptance gates. No Spotify FLAC access is added.

Preview.2 detects a disappeared bind address and cleans up before the user
LaunchAgent retries discovery, with a 30-second throttle. Manual startup validates
its interface/address and preserves LaunchAgent management when changing modes.
The 59-test native and instrumented suites include four isolated Bonjour lifecycle
tests. The separate real return-to-LAN observation confirms this recovery case;
it does not prove extended streaming stability or every network-change scenario.
Preview.3 adds bounded PCM submission/signal statistics, retains pending PCM when
the sink is down or rejects a chunk, and keeps connection calls independent of
logging. Four additional tests compare signed PCM through the exact ALAC encoders
and Apple's reference decoder. ALAC and raw ALAC local probes each submitted
132,300 frames with receiver timing replies and no observed send errors; audible
output remains unconfirmed. Newer AirPlay 2 clock compatibility is under separate
investigation; it is not an implemented Spotify FLAC provider.

Use the install commands above or scripts/build-macos.sh. Installation is user-only,
verifies hashes and ad-hoc signatures, and adds a user LaunchAgent. Runtime secrets
stay outside Git. LAN selection binds only this bridge; system networking is unchanged.

Audit/test/acceptance documents contain precise evidence and outstanding gates. MIT
SpotConnect links GPL-3.0-or-later cspot; the combined executable is distributed under
GPL-3.0-or-later with original notices and dependency licenses retained. No Spotify
certification or HomePod end-to-end bit-perfect behavior is claimed.
