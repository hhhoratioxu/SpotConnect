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
RAOP 握手及原创本地 FLAC 的 PCM 发送；可听结果、Spotify 账号播放和控制验收
仍标记 UNVERIFIED。

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
| HomePod 2 发现 | 当前 LAN 实测 PASS；原生 Bonjour 工具和桥接发现均成功 |
| Connect 服务与设备信息 | 实测注册，ZeroConf 接口返回正常；Spotify UI 确认 UNVERIFIED |
| RAOP 输出 | 当前 HomePod 接受 AppleLossless/ALAC 会话，44.1 kHz / 16-bit / stereo |
| 本地 FLAC → PCM → RAOP | 实际提交 44,100 帧；低幅度测试，可听输出 UNVERIFIED |
| Spotify → HomePod 音乐 | UNVERIFIED；账号授权、个人 Client ID/Secret 尚未配置验证 |
| 暂停 / Seek / 切歌 / 音量 | 控制路径保留；离线 Seek 测试 PASS，真实 Spotify 控制 UNVERIFIED |
| 长时间连续播放 | UNVERIFIED |
| HomePod 端到端 bit-perfect | UNVERIFIED；24→16 位和重采样会改变样本 |
| 签名 | 本地 ad-hoc 签名验证；无 Developer ID 公证 |

### 安装与运行

下载 [Preview 发布包](https://github.com/hhhoratioxu/SpotConnect/releases/tag/v0.1.0-preview.1)，
解压并运行 scripts/install.sh。只写入当前用户目录，不需要 sudo。

```sh
curl -fL https://github.com/hhhoratioxu/SpotConnect/releases/download/v0.1.0-preview.1/HoriSpotConnect-0.1.0-preview.1-macos-arm64.zip -o HoriSpotConnect.zip
ditto -xk HoriSpotConnect.zip .
bash HoriSpotConnect-0.1.0-preview.1-macos-arm64/scripts/install.sh
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

启动器依据 HomePod 的 Bonjour 接口选择 LAN，避免误选本机虚拟网络接口。
只绑定本程序；可用 HORISPOT_INTERFACE=en7 等覆盖，未修改系统代理或网络配置。

### Spotify 登录与真实限制

在官方 Spotify 应用中选择名字后带 [HoriSpot] 的设备，以 upstream ZeroConf
方式登录。没有复制官方应用的令牌、密码或私钥。

上游自 2025-12 起在自编译版本中要求自己的 application Client ID 和 Client
Secret 获取其 Web API token；将自己的信息只填入安装目录 state/config.xml
的 <client_id> / <client_secret>，该文件权限为 0600。这些应用凭据不是 Premium
账号的 Lossless 授权，也不能解锁 FLAC。缺少凭据时清晰报错，不发送占位符或
打印秘密。

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
当前完整套件 **44/44 PASS**，地址/未定义行为检查 **44/44 PASS**；覆盖原始
PCM 逐样本比较、CDN/AES/HTTP Range 路径、Vorbis 96/160/320 解码与 Seek。
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
with deliberate attenuation for the short test. Audible output, Spotify Premium
playback, and controls remain **UNVERIFIED**. UI testing was blocked by the locked
Mac, and personal application credentials were not supplied.

Use the install commands above or scripts/build-macos.sh. Installation is user-only,
verifies hashes and ad-hoc signatures, and adds a user LaunchAgent. Runtime secrets
stay outside Git. LAN selection binds only this bridge; system networking is unchanged.

Audit/test/acceptance documents contain precise evidence and outstanding gates. MIT
SpotConnect links GPL-3.0-or-later cspot; the combined executable is distributed under
GPL-3.0-or-later with original notices and dependency licenses retained. No Spotify
certification or HomePod end-to-end bit-perfect behavior is claimed.
