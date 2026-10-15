// Copyright (C) 2026 hhhoratioxu. GPL-3.0-or-later.
// Exercise the exact prebuilt ALAC encoders linked by the RAOP sender, using
// Apple's reference decoder and independent signed PCM as the oracle.
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "ALACDecoder.h"
#include "ALACBitUtilities.h"
#include "alac_wrapper.h"

static void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int main(int argc, char** argv) {
  try {
    require(argc == 3, "usage: alac-roundtrip alac|alac_raw full|short");
    const bool raw = std::string(argv[1]) == "alac_raw";
    require(raw || std::string(argv[1]) == "alac", "unknown codec");
    const bool shortPacket = std::string(argv[2]) == "short";
    require(shortPacket || std::string(argv[2]) == "full", "unknown packet size");
    constexpr unsigned block = 352;
    const unsigned sourceFrames = shortPacket ? 23 : block;
    std::unique_ptr<alac_codec_s, decltype(&alac_delete_encoder)> encoder(
        alac_create_encoder(block, 44100, 16, 2), alac_delete_encoder);
    require(static_cast<bool>(encoder), "encoder creation failed");
    const std::array<std::int16_t, 8> extrema = {-32768, 32767, -1, 0, 1, 12345, -23456, 17};
    for (unsigned packetIndex = 0; packetIndex < 7; ++packetIndex) {
      // Padding accommodates the upstream encoder's documented input lookahead.
      std::vector<std::int16_t> source(block * 4, 0);
      for (unsigned frame = 0; frame < sourceFrames; ++frame) {
        source[2 * frame] = extrema[(frame + packetIndex) % extrema.size()];
        source[2 * frame + 1] = static_cast<std::int16_t>(
            static_cast<int>((frame * 431 + packetIndex * 1709) % 65536) - 32768);
      }
      uint8_t* allocated = nullptr;
      int encodedBytes = 0;
      const bool success = raw
          ? pcm_to_alac_raw(reinterpret_cast<uint8_t*>(source.data()), sourceFrames,
                            &allocated, &encodedBytes, block)
          : pcm_to_alac(encoder.get(), reinterpret_cast<uint8_t*>(source.data()),
                        sourceFrames, &allocated, &encodedBytes);
      std::unique_ptr<uint8_t, decltype(&std::free)> encoded(allocated, std::free);
      require(success && encoded && encodedBytes > 0, "ALAC encode failed");
      // The cookie describes the actual RAOP SDP: 352 frames, 16-bit stereo,
      // pb=40, mb=10, kb=14, maxRun=255, sampleRate=44100, network byte order.
      std::array<uint8_t, 24> cookie = {
          0, 0, 1, 96, 0, 16, 40, 10, 14, 2, 0, 255,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 172, 68};
      ALACDecoder decoder;
      require(decoder.Init(cookie.data(), cookie.size()) == 0, "reference decoder init failed");
      BitBuffer bits;
      BitBufferInit(&bits, encoded.get(), encodedBytes);
      std::vector<std::int16_t> decoded(block * 2, 0);
      uint32_t outputFrames = 0;
      require(decoder.Decode(&bits, reinterpret_cast<uint8_t*>(decoded.data()),
                             block, 2, &outputFrames) == 0, "reference decode failed");
      require(outputFrames == (raw ? block : sourceFrames), "unexpected decoded frame count");
      require(std::equal(source.begin(), source.begin() + sourceFrames * 2,
                         decoded.begin()), "ALAC changed original PCM samples");
      if (raw && shortPacket)
        require(std::all_of(decoded.begin() + sourceFrames * 2, decoded.end(),
                            [](std::int16_t sample) { return sample == 0; }),
                "short raw ALAC packet has nonzero padding");
    }
    std::cout << "Exact signed PCM roundtrip passed; no physical receiver claim.\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "ALAC_ROUNDTRIP_FAILED " << error.what() << '\n';
    return 1;
  }
}
