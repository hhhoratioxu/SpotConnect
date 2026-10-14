// Copyright (C) 2026 hhhoratioxu. MIT License; see the repository LICENSE.
#include <dns_sd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <net/if.h>
#include <chrono>
#include <charconv>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>

static std::string json(const std::string& value) {
  std::string out = "\"";
  constexpr char hex[] = "0123456789abcdef";
  for (const unsigned char c : value) {
    if (c == '"' || c == '\\') { out += '\\'; out += c; }
    else if (c < 32) { out += "\\u00"; out += hex[c >> 4]; out += hex[c & 15]; }
    else out += c;
  }
  return out + '"';
}
struct Entry { DNSServiceRef ref = nullptr; std::string name, type, domain; };
struct Discovery {
  std::vector<std::unique_ptr<Entry>> entries;
  std::set<std::string> seen;
  std::vector<std::string> results;
  std::string match;
  bool callbackError = false;
  bool spotify = false;
};
static void resolved(DNSServiceRef, DNSServiceFlags, uint32_t interface, DNSServiceErrorType error,
                     const char* full, const char* host, uint16_t port, uint16_t length,
                     const unsigned char* txt, void* opaque) noexcept {
  auto& d = *static_cast<Discovery*>(opaque);
  try {
    if (error || !full || !host || !d.seen.insert(full).second) return;
    char interfaceName[IF_NAMESIZE] = {};
    if_indextoname(interface, interfaceName);
    std::string out = "{\"service\":" + json(full) + ",\"host\":" + json(host) +
        ",\"interface\":" + json(interfaceName) +
        ",\"port\":" + std::to_string(ntohs(port)) + ",\"advertised\":{";
    bool comma = false;
    for (const auto* key : {"am", "cn", "sr", "ss", "ch", "et", "VERSION", "CPath"}) {
      uint8_t size = 0;
      const void* p = TXTRecordGetValuePtr(length, txt, key, &size);
      if (!p) continue;
      if (comma) out += ',';
      out += json(key) + ":" + json(std::string(static_cast<const char*>(p), size));
      comma = true;
    }
    out += "},\"audio_delivery_verified\":false,\"bit_perfect_verified\":false}";
    d.results.push_back(out);
  } catch (...) { d.callbackError = true; }
}
static void browsed(DNSServiceRef, DNSServiceFlags flags, uint32_t interface,
                    DNSServiceErrorType error, const char* name, const char* type,
                    const char* domain, void* opaque) noexcept {
  auto& d = *static_cast<Discovery*>(opaque);
  try {
    if (error || !(flags & kDNSServiceFlagsAdd) || !name ||
        (!d.match.empty() && std::string(name).find(d.match) == std::string::npos)) return;
    auto entry = std::make_unique<Entry>();
    if (DNSServiceResolve(&entry->ref, 0, interface, name, type, domain, resolved, &d) !=
        kDNSServiceErr_NoError) return;
    d.entries.push_back(std::move(entry));
  } catch (...) { d.callbackError = true; }
}
int main(int argc, char** argv) {
  Discovery d;
  int seconds = 5;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if ((arg == "--seconds" || arg == "--match") && i + 1 < argc) {
      const std::string value = argv[++i];
      if (arg == "--match") d.match = value;
      else {
        auto r = std::from_chars(value.data(), value.data() + value.size(), seconds);
        if (r.ec != std::errc{} || r.ptr != value.data() + value.size() || seconds < 1 || seconds > 60) {
          std::cerr << "--seconds must be 1..60\n"; return 2;
        }
      }
    } else if (arg == "--spotify") d.spotify = true;
    else if (arg == "--help") {
      std::cout << "hori-discover [--seconds 1..60] [--match NAME] [--spotify]\n"
                   "Read-only Bonjour discovery. TXT values are advertisements, not playback proof.\n";
      return 0;
    } else { std::cerr << "invalid argument; use --help\n"; return 2; }
  }
  const std::vector<const char*> types = d.spotify ?
      std::vector<const char*>{"_spotify-connect._tcp"} :
      std::vector<const char*>{"_raop._tcp", "_airplay._tcp"};
  for (auto type : types) {
    auto entry = std::make_unique<Entry>();
    if (DNSServiceBrowse(&entry->ref, 0, 0, type, "local", browsed, &d) != kDNSServiceErr_NoError) {
      for (auto& item : d.entries) DNSServiceRefDeallocate(item->ref);
      std::cerr << "BONJOUR_BROWSE_ERROR\n"; return 1;
    }
    d.entries.push_back(std::move(entry));
  }
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
  while (std::chrono::steady_clock::now() < deadline && !d.callbackError) {
    // Callbacks may append resolvers; snapshot descriptors before processing.
    std::vector<pollfd> descriptors;
    const auto count = d.entries.size();
    for (std::size_t i = 0; i < count; ++i)
      descriptors.push_back({DNSServiceRefSockFD(d.entries[i]->ref), POLLIN, 0});
    if (poll(descriptors.data(), descriptors.size(), 100) > 0)
      for (std::size_t i = 0; i < count; ++i)
        if (descriptors[i].revents & POLLIN)
          if (DNSServiceProcessResult(d.entries[i]->ref) != kDNSServiceErr_NoError)
            d.callbackError = true;
  }
  for (auto& entry : d.entries) DNSServiceRefDeallocate(entry->ref);
  if (d.callbackError) { std::cerr << "BONJOUR_PROCESS_ERROR\n"; return 1; }
  std::cout << "{\"devices\":[";
  for (std::size_t i = 0; i < d.results.size(); ++i) {
    if (i) std::cout << ',';
    std::cout << d.results[i];
  }
  std::cout << "],\"count\":" << d.results.size() << "}\n";
  return 0;
}
