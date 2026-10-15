// Copyright (C) 2026 hhhoratioxu. MIT; see LICENSE.
// This target redirects only HoriBonjour's getifaddrs reads. It never changes
// interfaces, routes, Wi-Fi or the system Bonjour service.
#include "../HoriBonjour.h"
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <chrono>
#include <iostream>
#include <string>

#undef getifaddrs
extern "C" int getifaddrs(ifaddrs**);
static int reads = 0, liveReads = 10000;
static std::chrono::steady_clock::time_point stopAt;
extern "C" int hori_test_getifaddrs(ifaddrs** addresses) {
  if (++reads > liveReads) { *addresses = nullptr; return 0; }
  return getifaddrs(addresses);
}
static bool running() { return std::chrono::steady_clock::now() < stopAt; }
static bool received(mdnssd_service_t*, void*, bool*) { return false; }
static void require(bool value, const char* reason) {
  if (!value) { std::cerr << reason << '\n'; std::exit(1); }
}
int main(int argc, char** argv) {
  require(argc == 2, "one test case required");
  const std::string test = argv[1];
  const auto start = std::chrono::steady_clock::now();
  stopAt = start + std::chrono::seconds(6);
  in_addr bind{};
  require(inet_pton(AF_INET, "127.0.0.1", &bind) == 1, "loopback address");
  if (test == "missing") liveReads = 0;
  else if (test == "lost") liveReads = 2;  // Live at browse and first poll; lost later.
  else if (test == "stop") stopAt = start + std::chrono::milliseconds(250);
  else require(test == "bounded", "unknown case");
  const bool ok = hori_bonjour_query(bind, test == "bounded" ? 1 : 0,
                                    received, nullptr, running);
  const auto elapsed = std::chrono::steady_clock::now() - start;
  if (test == "missing") {
    require(!ok && elapsed < std::chrono::seconds(1), "missing LAN must fail promptly");
  } else if (test == "lost") {
    require(!ok && reads >= 3 && elapsed >= std::chrono::milliseconds(1900) &&
            elapsed < std::chrono::seconds(5), "live browse must detect a disappearing bind");
  } else {
    require(ok, "bounded query and explicit stop must finish normally");
    require(elapsed < std::chrono::seconds(3), "stop/deadline must release the query");
  }
  std::cout << "PASS " << test << " simulated interface lifecycle; no physical LAN changes\n";
}
