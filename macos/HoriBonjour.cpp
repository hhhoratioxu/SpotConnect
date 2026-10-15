// Copyright (C) 2026 hhhoratioxu. MIT; see LICENSE.
#include "HoriBonjour.h"
#include <dns_sd.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <poll.h>
#include <chrono>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace {
static uint32_t interfaceForAddress(in_addr bind) {
  uint32_t interface = 0;
  ifaddrs* addresses = nullptr;
  if (getifaddrs(&addresses) == 0) {
    for (auto* item = addresses; item; item = item->ifa_next)
      if (item->ifa_addr && item->ifa_addr->sa_family == AF_INET &&
          (item->ifa_flags & IFF_UP) &&
          reinterpret_cast<sockaddr_in*>(item->ifa_addr)->sin_addr.s_addr == bind.s_addr)
        interface = if_nametoindex(item->ifa_name);
    freeifaddrs(addresses);
  }
  return interface;
}
struct Query;
struct Record {
  Query* query = nullptr;
  DNSServiceRef resolve = nullptr, address = nullptr;
  std::string instance, type, domain, full, hostname;
  std::vector<std::pair<std::string, std::string>> attributes;
  uint32_t interface = 0;
  uint16_t port = 0;
  in_addr ip{};
  bool announced = false, removed = false;
  bool restartAddress = false;
  ~Record() {
    if (resolve) DNSServiceRefDeallocate(resolve);
    if (address) DNSServiceRefDeallocate(address);
  }
};
struct Query {
  mdns_callback_t* callback;
  void* cookie;
  std::vector<std::unique_ptr<Record>> records;
  bool failed = false;
  void deliver(Record& record, bool expired) {
    std::vector<mdnssd_txt_attr_t> attributes;
    for (auto& item : record.attributes) attributes.push_back({item.first.data(), item.second.data()});
    mdnssd_service_t service{};
    service.host = service.addr = record.ip; // Actual resolved receiver IPv4.
    service.name = record.full.data();
    service.hostname = record.hostname.data();
    service.port = record.port;
    service.expired = expired;
    service.since = expired ? 0 : 1;
    service.attr = attributes.data();
    service.attr_count = static_cast<int>(attributes.size());
    bool stop = false;
    callback(&service, cookie, &stop);
    if (stop) failed = true;
  }
};
static void gotAddress(DNSServiceRef, DNSServiceFlags flags, uint32_t,
                       DNSServiceErrorType error, const char*, const sockaddr* address,
                       uint32_t, void* opaque) noexcept {
  auto& record = *static_cast<Record*>(opaque);
  try {
    if (error || record.removed || !(flags & kDNSServiceFlagsAdd) || !address ||
        address->sa_family != AF_INET) return;
    const auto ip = reinterpret_cast<const sockaddr_in*>(address)->sin_addr;
    if (!ip.s_addr) return;
    record.ip = ip;
    record.query->deliver(record, false);
    record.announced = true;
  } catch (...) { record.query->failed = true; }
}
static void resolved(DNSServiceRef, DNSServiceFlags, uint32_t interface,
                     DNSServiceErrorType error, const char* full, const char* host,
                     uint16_t port, uint16_t size, const unsigned char* txt,
                     void* opaque) noexcept {
  auto& record = *static_cast<Record*>(opaque);
  try {
    if (error || record.removed || !full || !host) return;
    record.full = full; record.hostname = host; record.port = ntohs(port);
    record.attributes.clear();
    for (uint16_t i = 0; i < TXTRecordGetCount(size, txt); ++i) {
      char key[256]{}; uint8_t length = 0; const void* value = nullptr;
      if (TXTRecordGetItemAtIndex(size, txt, i, sizeof(key), key, &length, &value) == kDNSServiceErr_NoError)
        record.attributes.emplace_back(key, value ? std::string(static_cast<const char*>(value), length) : "");
    }
    if (record.address) {
      // Do not invalidate a reference still held by this iteration's poll snapshot.
      record.restartAddress = true;
      return;
    }
    if (DNSServiceGetAddrInfo(&record.address, 0, interface, kDNSServiceProtocol_IPv4,
                              host, gotAddress, &record) != kDNSServiceErr_NoError)
      record.query->failed = true;
  } catch (...) { record.query->failed = true; }
}
static void browsed(DNSServiceRef, DNSServiceFlags flags, uint32_t interface,
                    DNSServiceErrorType error, const char* name, const char* type,
                    const char* domain, void* opaque) noexcept {
  auto& query = *static_cast<Query*>(opaque);
  try {
    if (error || !name || !type || !domain) return;
    for (auto& record : query.records) {
      if (!record->removed && record->instance == name && record->type == type &&
          record->domain == domain && record->interface == interface) {
        if (!(flags & kDNSServiceFlagsAdd)) {
          record->removed = true;
          if (record->announced) query.deliver(*record, true);
        }
        return;
      }
    }
    if (!(flags & kDNSServiceFlagsAdd)) return;
    if (query.records.size() >= 512) return;
    auto record = std::make_unique<Record>();
    record->query = &query; record->instance = name; record->type = type;
    record->domain = domain; record->interface = interface;
    if (DNSServiceResolve(&record->resolve, 0, interface, name, type, domain,
                          resolved, record.get()) != kDNSServiceErr_NoError) return;
    query.records.push_back(std::move(record));
  } catch (...) { query.failed = true; }
}
}  // namespace
extern "C" bool hori_bonjour_query(in_addr bind, int seconds, mdns_callback_t* callback,
                                   void* cookie, bool (*running)(void)) {
  if (!callback || !running) return false;
  Query query{callback, cookie, {}, false};
  DNSServiceRef browse = nullptr;
  const auto interface = interfaceForAddress(bind);
  if (!interface || DNSServiceBrowse(&browse, 0, interface, "_raop._tcp", "local",
                                     browsed, &query) != kDNSServiceErr_NoError) return false;
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
  auto nextBindCheck = std::chrono::steady_clock::now();
  while (running() && !query.failed && (!seconds || std::chrono::steady_clock::now() < deadline)) {
    if (std::chrono::steady_clock::now() >= nextBindCheck) {
      nextBindCheck = std::chrono::steady_clock::now() + std::chrono::seconds(2);
      if (interfaceForAddress(bind) != interface) {
        std::fprintf(stderr, "HORI_LAN_BIND_LOST: selected interface/address disappeared; rediscovery required\n");
        query.failed = true;
        break;
      }
    }
    std::vector<DNSServiceRef> refs{browse};
    for (auto& record : query.records) if (!record->removed) {
      if (record->resolve) refs.push_back(record->resolve);
      if (record->address) refs.push_back(record->address);
    }
    std::vector<pollfd> descriptors;
    for (auto ref : refs) descriptors.push_back({DNSServiceRefSockFD(ref), POLLIN, 0});
    if (poll(descriptors.data(), descriptors.size(), 100) > 0)
      for (std::size_t i = 0; i < refs.size(); ++i)
        if (descriptors[i].revents & POLLIN)
          if (DNSServiceProcessResult(refs[i]) != kDNSServiceErr_NoError) query.failed = true;
    for (auto& record : query.records) if (record->restartAddress && !record->removed) {
      DNSServiceRefDeallocate(record->address);
      record->address = nullptr;
      record->restartAddress = false;
      if (DNSServiceGetAddrInfo(&record->address, 0, record->interface,
          kDNSServiceProtocol_IPv4, record->hostname.c_str(), gotAddress, record.get()) != kDNSServiceErr_NoError)
        query.failed = true;
    }
    std::erase_if(query.records, [](const auto& record) { return record->removed; });
  }
  DNSServiceRefDeallocate(browse);
  if (query.failed) std::fprintf(stderr, "HORI_BONJOUR_DISCOVERY_ERROR\n");
  return !query.failed;
}
