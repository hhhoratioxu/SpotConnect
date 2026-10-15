// Copyright (C) 2026 hhhoratioxu. GPL-3.0-or-later (links cspot audio).
#include "FlacDecoder.h"
#include "RaopPcmReader.h"
extern "C" {
#include "raop_client.h"
#include "cross_ssl.h"
#include "cross_net.h"
#include "cross_log.h"
}
#include <chrono>
#include <charconv>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
extern "C" { log_level raop_loglevel = lINFO, util_loglevel = lINFO; }
int main(int argc, char** argv) {
  struct raopcl_s* sender = nullptr;
  bool ssl = false, connected = false;
  try {
    std::string bind, host, file, model, codecName = "ALAC";
    raop_codec_t codec = RAOP_ALAC;
    unsigned port = 7000;
    for (int i = 1; i < argc; ++i) {
      const std::string arg = argv[i];
      if (arg == "--help") {
        std::cout << "hori-raop-probe --bind IPV4 --host IPV4 --model ADVERTISED_MODEL [--port N] [--local-flac FILE] [--codec alac|alac_raw] [--verbose]\n"
                     "Default: test only the RAOP session, preserve device volume, then disconnect.\n"
                     "Optional local FLAC sends at most 3 seconds at reduced sample amplitude.\n"
                     "No Spotify source or end-to-end bit-perfect claim.\n"; return 0;
      }
      if (arg == "--verbose") { raop_loglevel = lDEBUG; continue; }
      if (i + 1 >= argc) throw std::invalid_argument("argument needs a value");
      const std::string value = argv[++i];
      if (arg == "--bind") bind = value;
      else if (arg == "--host") host = value;
      else if (arg == "--model") model = value;
      else if (arg == "--local-flac") file = value;
      else if (arg == "--codec") {
        if (value == "alac") { codec = RAOP_ALAC; codecName = "ALAC"; }
        else if (value == "alac_raw") { codec = RAOP_ALAC_RAW; codecName = "ALAC_RAW"; }
        else throw std::invalid_argument("codec must be alac or alac_raw (both AppleLossless)");
      }
      else if (arg == "--port") {
        const auto r = std::from_chars(value.data(), value.data() + value.size(), port);
        if (r.ec != std::errc{} || r.ptr != value.data() + value.size() || port < 1 || port > 65535)
          throw std::invalid_argument("invalid port");
      } else throw std::invalid_argument("invalid argument");
    }
    in_addr local{}, peer{};
    if (inet_pton(AF_INET, bind.c_str(), &local) != 1 || inet_pton(AF_INET, host.c_str(), &peer) != 1 || model.empty())
      throw std::invalid_argument("require literal LAN IPv4 addresses and the actual advertised model");
    std::unique_ptr<cspot::audio::FlacDecoder> decoder;
    std::unique_ptr<cspot::audio::RaopPcmReader> pcm;
    if (!file.empty()) {
      decoder = std::make_unique<cspot::audio::FlacDecoder>(file);
      pcm = std::make_unique<cspot::audio::RaopPcmReader>(*decoder);
    }
    ssl = cross_ssl_load();
    if (!ssl) throw std::runtime_error("OpenSSL initialization failed");
    char et[] = "0,3,5";
    sender = raopcl_create(local, 0, 1, nullptr, nullptr, codec, 352, 44100,
                           RAOP_CLEAR, false, nullptr, nullptr, et, nullptr,
                           model.c_str(), 44100, 16, 2, -40.0f);
    if (!sender) throw std::runtime_error("RAOP sender creation failed");
    connected = raopcl_connect(sender, peer, port, false); // Never set the device's volume.
    if (!connected) throw std::runtime_error("HomePod rejected or did not complete this RAOP session");
    std::uint64_t frames = 0;
    if (pcm) {
      const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(8);
      std::vector<std::int16_t> pending;
      std::size_t offset = 0;
      while (frames < 44100 * 3 && std::chrono::steady_clock::now() < deadline) {
        if (pending.empty() || offset == pending.size()) { pending = pcm->read(); offset = 0; }
        if (pending.empty()) break;
        if (!raopcl_accept_frames(sender)) { std::this_thread::sleep_for(std::chrono::milliseconds(5)); continue; }
        const auto count = std::min<std::size_t>(
            std::min<std::size_t>(352, (pending.size() - offset) / 2), 44100 * 3 - frames);
        std::vector<std::int16_t> quiet(count * 2);
        for (std::size_t i = 0; i < quiet.size(); ++i) quiet[i] = pending[offset + i] / 64;
        std::uint64_t playtime = 0;
        if (!raopcl_send_chunk(sender, reinterpret_cast<uint8_t*>(quiet.data()), count, &playtime))
          throw std::runtime_error("RAOP sample send failed");
        frames += count; offset += quiet.size();
      }
      if (!frames) throw std::runtime_error("RAOP accepted no audio frames");
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    const bool senderSane = raopcl_is_sane(sender);
    raopcl_disconnect(sender); raopcl_destroy(sender); sender = nullptr;
    cross_ssl_free(); ssl = false;
    std::cout << "{\"raop_session_accepted\":true,\"source\":"
              << (pcm ? "\"local_FLAC\"" : "\"none_connect_only\"")
              << ",\"wire_codec\":\"AppleLossless\",\"encoder_requested\":\"" << codecName
              << "\",\"sender_sane\":" << (senderSane ? "true" : "false")
              << ",\"sample_rate\":44100,\"bits\":16,\"channels\":2,"
                 "\"frames_submitted\":" << frames
              << ",\"samples_attenuated\":" << (pcm ? "true" : "false");
    if (decoder) {
      const auto& format = decoder->format();
      std::cout << ",\"source_sample_rate\":" << format.sampleRate
                << ",\"source_bits\":" << format.bitsPerSample
                << ",\"source_channels\":" << format.channels;
    }
    std::cout << ",\"spotify_flac_verified\":false,\"audible_output_verified\":false,\"bit_perfect_verified\":false}\n";
    return 0;
  } catch (const std::exception& e) {
    if (sender) { raopcl_disconnect(sender); raopcl_destroy(sender); }
    if (ssl) cross_ssl_free();
    std::cerr << "RAOP_PROBE_ERROR " << e.what() << '\n'; return 1;
  }
}
