// Copyright (C) 2026 hhhoratioxu. MIT; see LICENSE.
// PCM/control adapter to Music Assistant's GPL-3.0 airplay-cli. No Spotify
// credentials, pairing secrets or raw child diagnostics are logged here.
#ifdef __APPLE__
extern "C" {
#include "raop_client.h"
}
#define HORI_OUTPUT_IMPLEMENTATION
#include "HoriAirplayOutput.h"
#include <mach-o/dyld.h>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <poll.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

extern char** environ;
namespace {
using Clock = std::chrono::steady_clock;
constexpr size_t capacity = 44100 * 4 * 2;
constexpr uint64_t startLeadMs = 4000;
uint64_t wallMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}
void nonblocking(int fd) { fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK); }
void closeFd(int& fd) { if (fd >= 0) close(fd); fd = -1; }
std::string helperPath() {
    if (const char* path = std::getenv("HORISPOT_AIRPLAY2_SENDER")) return path;
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::vector<char> buffer(size + 1);
    if (_NSGetExecutablePath(buffer.data(), &size)) return {};
    return (std::filesystem::path(buffer.data()).parent_path() / "hori-airplay").string();
}
struct NativeOutput {
    std::mutex lifecycle, mutex;
    std::condition_variable changed;
    std::thread worker;
    bool running = false, connected = false, paused = false, started = false;
    bool startSent = false, buffered = false, flushing = false, failed = false;
    int input = -1, output = -1, control = -1;
    pid_t child = -1;
    std::string directory, fifo, bind;
    std::deque<std::vector<uint8_t>> queue;
    size_t queued = 0, offset = 0;
    uint64_t frames = 0, progressOffset = 0, duration = 0;
    Clock::time_point audibleAt{};
    unsigned volume = 20;

    explicit NativeOutput(in_addr local) {
        char address[INET_ADDRSTRLEN]{};
        inet_ntop(AF_INET, &local, address, sizeof(address)); bind = address;
    }
    ~NativeOutput() { disconnect(); }
    bool command(const std::string& value) { // mutex is held; commands fit PIPE_BUF.
        return control >= 0 && write(control, value.data(), value.size()) == static_cast<ssize_t>(value.size());
    }
    uint64_t played() const {
        if (!started || Clock::now() <= audibleAt) return 0;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - audibleAt).count();
        return std::min<uint64_t>(frames, static_cast<uint64_t>(ms) * 44100 / 1000);
    }
    void status(const std::string& line) { // mutex is held.
        if (line.rfind("[STATUS] connected", 0) == 0) {
            connected = true; changed.notify_all();
            std::fprintf(stderr, "AIRPLAY2_STATUS state=connected timing=NTP PCM=s16le/44100/2 audible_output=UNVERIFIED\n");
        } else if (line.rfind("[STATUS] audio buffered_ms=", 0) == 0) buffered = true;
        else if (line.rfind("[STATUS] flushed", 0) == 0) {
            flushing = false; buffered = false; changed.notify_all();
        } else if (line.rfind("[STATUS] started ", 0) == 0) {
            started = true;
            std::fprintf(stderr, "AIRPLAY2_STATUS state=started audible_output=UNVERIFIED\n");
        } else if (line.rfind("[STATUS] error", 0) == 0) {
            failed = true; changed.notify_all();
            std::fprintf(stderr, "AIRPLAY2_ERROR sender_reported_error PCM_pending=retained\n");
        }
    }
    void run() {
        std::string pending;
        while (true) {
            // Only this worker reads this descriptor; cleanup closes it after
            // joining. Drain outside the shared state lock, even if an OS call
            // were unexpectedly slow despite O_NONBLOCK.
            std::vector<std::string> lines;
            char bytes[8192]; ssize_t count;
            while ((count = read(output, bytes, sizeof(bytes))) > 0) {
                pending.append(bytes, count);
                size_t newline;
                while ((newline = pending.find('\n')) != std::string::npos) {
                    if (newline <= 4096) lines.push_back(pending.substr(0, newline));
                    pending.erase(0, newline + 1);
                }
                if (pending.size() > 65536) pending.clear();
            }
            {
                std::unique_lock lock(mutex);
                if (!running) break;
                for (const auto& line : lines) status(line);
                int exitStatus = 0;
                if (waitpid(child, &exitStatus, WNOHANG) == child) {
                    child = -1; running = connected = false; failed = true;
                    changed.notify_all();
                    std::fprintf(stderr, "AIRPLAY2_ERROR child_exited PCM_pending=retained\n");
                    break;
                }
                if (connected && !paused && !flushing && !queue.empty()) {
                    const auto& block = queue.front();
                    auto written = write(input, block.data() + offset, block.size() - offset);
                    if (written > 0) {
                        offset += written; queued -= written;
                        if (offset == block.size()) { queue.pop_front(); offset = 0; }
                    } else if (written < 0 && errno != EAGAIN && errno != EINTR) {
                        failed = true;
                    }
                }
                if (connected && buffered && frames && !paused && !flushing && !startSent && !failed) {
                    const auto when = wallMs() + startLeadMs;
                    startSent = command("START_UNIX_MS=" + std::to_string(when) + "\nACTION=START\n");
                    audibleAt = Clock::now() + std::chrono::milliseconds(startLeadMs);
                }
            }
            pollfd fd{output, POLLIN, 0}; poll(&fd, 1, 5);
        }
    }
    bool connect(in_addr peer, uint16_t port) {
        std::unique_lock operation(lifecycle);
        {
            std::lock_guard lock(mutex);
            if (running && connected && !failed) { paused = false; return true; }
        }
        cleanup();
        const auto helper = helperPath();
        if (helper.empty() || access(helper.c_str(), X_OK)) {
            std::fprintf(stderr, "AIRPLAY2_ERROR native_sender_missing\n"); return false;
        }
        char temp[] = "/tmp/horispot-ap2.XXXXXX";
        const char* path = mkdtemp(temp);
        if (!path) return false;
        directory = path; fifo = directory + "/control";
        if (mkfifo(fifo.c_str(), 0600)) { cleanup(); return false; }
        control = open(fifo.c_str(), O_RDWR | O_NONBLOCK | O_CLOEXEC);
        int audio[2]{-1,-1}, logs[2]{-1,-1};
        if (control < 0 || pipe(audio) || pipe(logs)) {
            for (int fd : audio) if (fd >= 0) close(fd);
            for (int fd : logs) if (fd >= 0) close(fd);
            cleanup(); return false;
        }
        char address[INET_ADDRSTRLEN]{};
        inet_ntop(AF_INET, &peer, address, sizeof(address));
        std::vector<std::string> args{helper, "--protocol", "airplay2", "--ap2-native",
            "--timing", "ntp", "--port", std::to_string(port), "--if", bind,
            "--volume", std::to_string(volume), "--samplerate", "44100", "--bitdepth", "16",
            "--channels", "2", "--cmdpipe", fifo, address};
        std::vector<char*> argv;
        for (auto& arg : args) argv.push_back(arg.data()); argv.push_back(nullptr);
        posix_spawn_file_actions_t actions; posix_spawn_file_actions_init(&actions);
        posix_spawn_file_actions_adddup2(&actions, audio[0], STDIN_FILENO);
        posix_spawn_file_actions_adddup2(&actions, logs[1], STDOUT_FILENO);
        posix_spawn_file_actions_adddup2(&actions, logs[1], STDERR_FILENO);
        posix_spawnattr_t attributes; posix_spawnattr_init(&attributes);
        posix_spawnattr_setflags(&attributes, POSIX_SPAWN_CLOEXEC_DEFAULT);
        const int error = posix_spawn(&child, helper.c_str(), &actions, &attributes, argv.data(), environ);
        posix_spawn_file_actions_destroy(&actions); posix_spawnattr_destroy(&attributes);
        close(audio[0]); close(logs[1]);
        input = audio[1]; output = logs[0];
        if (error) { child = -1; cleanup(); return false; }
        nonblocking(input); nonblocking(output);
        { std::lock_guard lock(mutex); running = true; paused = failed = false; }
        worker = std::thread([this]{ run(); });
        std::unique_lock lock(mutex);
        changed.wait_for(lock, std::chrono::seconds(6), [this]{return connected || failed;});
        const bool ok = connected && !failed;
        lock.unlock();
        if (!ok) cleanup();
        return ok;
    }
    void cleanup() { // lifecycle is held; the worker may still need mutex.
        { std::lock_guard lock(mutex); command("ACTION=DISCONNECT\n"); running = false; }
        if (worker.joinable()) worker.join();
        if (child > 0) {
            kill(child, SIGTERM);
            bool reaped = false;
            for (int i = 0; i < 100; ++i) {
                if (waitpid(child, nullptr, WNOHANG) == child) { reaped = true; break; }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            if (!reaped) { kill(child, SIGKILL); waitpid(child, nullptr, 0); }
            child = -1;
        }
        std::lock_guard lock(mutex);
        closeFd(input); closeFd(output); closeFd(control);
        if (!fifo.empty()) unlink(fifo.c_str());
        if (!directory.empty()) rmdir(directory.c_str());
        fifo.clear(); directory.clear(); queue.clear(); queued = offset = frames = 0;
        connected = started = startSent = buffered = flushing = false;
    }
    bool disconnect() { std::lock_guard operation(lifecycle); cleanup(); return true; }
    bool flush() {
        std::lock_guard operation(lifecycle);
        std::unique_lock lock(mutex);
        queue.clear(); queued = offset = frames = 0;
        started = startSent = buffered = false;
        if (!running || !connected) return true;
        flushing = true;
        if (!command("ACTION=FLUSH\n")) { failed = true; return false; }
        changed.wait_for(lock, std::chrono::seconds(3), [this]{return !flushing || failed;});
        if (flushing) failed = true;
        return !failed;
    }
    bool accept() {
        std::lock_guard lock(mutex);
        return connected && !failed && !paused && !flushing && queued + 1408 <= capacity &&
            frames < played() + 44100 * 2;
    }
    bool send(uint8_t* pcm, int count, uint64_t* time) {
        if (!pcm || count < 1 || count > MAX_FRAMES_PER_CHUNK || !time) return false;
        std::lock_guard lock(mutex);
        const size_t bytes = static_cast<size_t>(count) * 4;
        if (!connected || failed || paused || flushing || queued + bytes > capacity) return false;
        queue.emplace_back(pcm, pcm + bytes); queued += bytes; frames += count;
        *time = raopcl_get_ntp(nullptr) + MS2NTP(startLeadMs);
        return true;
    }
};
std::mutex registryMutex;
std::unordered_map<raopcl_s*, std::shared_ptr<NativeOutput>> outputs;
std::shared_ptr<NativeOutput> find(raopcl_s* p) {
    std::lock_guard lock(registryMutex); auto it = outputs.find(p);
    return it == outputs.end() ? nullptr : it->second;
}
}
extern "C" {
bool hori_output_register(raopcl_s* p, in_addr bind, const char* model) {
    const char* selected = std::getenv("HORISPOT_AIRPLAY_OUTPUT");
    if (!p || !model || std::strcmp(model, "AudioAccessory6,1") ||
        (selected && !std::strcmp(selected, "legacy"))) return false;
    std::lock_guard lock(registryMutex); outputs[p] = std::make_shared<NativeOutput>(bind);
    std::fprintf(stderr, "AIRPLAY_BACKEND protocol=AirPlay2 timing=NTP model=AudioAccessory6,1 source_lossless=UNVERIFIED\n");
    return true;
}
bool hori_output_native(raopcl_s* p) { return static_cast<bool>(find(p)); }
bool hori_output_connect(raopcl_s* p, in_addr ip, uint16_t port, bool volume) {
    if (auto n = find(p)) return n->connect(ip, port);
    return raopcl_connect(p, ip, port, volume);
}
bool hori_output_disconnect(raopcl_s* p) { if (auto n=find(p)) return n->disconnect(); return raopcl_disconnect(p); }
bool hori_output_destroy(raopcl_s* p) {
    std::shared_ptr<NativeOutput> n;
    { std::lock_guard lock(registryMutex); auto it=outputs.find(p); if(it!=outputs.end()){n=it->second;outputs.erase(it);} }
    if(n) n->disconnect(); return raopcl_destroy(p);
}
bool hori_output_flush(raopcl_s* p) { if(auto n=find(p))return n->flush();return raopcl_flush(p); }
bool hori_output_keepalive(raopcl_s* p) { if(auto n=find(p)){std::lock_guard lock(n->mutex);return n->connected&&!n->failed;}return raopcl_keepalive(p); }
bool hori_output_accept_frames(raopcl_s* p) { if(auto n=find(p))return n->accept();return raopcl_accept_frames(p); }
bool hori_output_send_chunk(raopcl_s* p,uint8_t* data,int frames,uint64_t* time){if(auto n=find(p))return n->send(data,frames,time);return raopcl_send_chunk(p,data,frames,time);}
raop_state_t hori_output_state(raopcl_s* p){if(auto n=find(p)){std::lock_guard lock(n->mutex);return !n->connected||n->failed?RAOP_DOWN:n->started?RAOP_STREAMING:RAOP_FLUSHED;}return raopcl_state(p);}
bool hori_output_is_sane(raopcl_s* p){if(auto n=find(p)){std::lock_guard lock(n->mutex);return n->connected&&!n->failed;}return raopcl_is_sane(p);}
bool hori_output_is_connected(raopcl_s* p){if(auto n=find(p)){std::lock_guard lock(n->mutex);return n->connected&&!n->failed;}return raopcl_is_connected(p);}
bool hori_output_is_playing(raopcl_s* p){if(auto n=find(p)){std::lock_guard lock(n->mutex);return n->connected&&!n->paused&&!n->failed&&(!n->started||n->played()<n->frames);}return raopcl_is_playing(p);}
void hori_output_pause(raopcl_s* p){if(auto n=find(p)){std::lock_guard lock(n->mutex);n->progressOffset+=n->played()*1000/44100;n->paused=true;n->command("ACTION=PAUSE\n");}else raopcl_pause(p);}
void hori_output_stop(raopcl_s* p){if(auto n=find(p)){std::lock_guard lock(n->mutex);n->command("ACTION=PAUSE\n");}else raopcl_stop(p);}
uint64_t hori_output_get_progress_ms(raopcl_s* p){if(auto n=find(p)){std::lock_guard lock(n->mutex);return n->progressOffset+n->played()*1000/44100;}return raopcl_get_progress_ms(p);}
bool hori_output_set_progress_ms(raopcl_s* p,uint32_t elapsed,uint32_t duration){if(auto n=find(p)){std::lock_guard lock(n->mutex);n->progressOffset=elapsed;n->duration=duration;return n->command("DURATION="+std::to_string(duration)+"\nPROGRESS="+std::to_string(elapsed)+"\n");}return raopcl_set_progress_ms(p,elapsed,duration);}
bool hori_output_set_volume(raopcl_s* p,float db){if(auto n=find(p)){std::lock_guard lock(n->mutex);n->volume=db<=-30?0:static_cast<unsigned>(std::clamp((db+30)*100/30,0.0f,100.0f));return !n->connected||n->command("VOLUME="+std::to_string(n->volume)+"\n");}return raopcl_set_volume(p,db);}
}
#endif
