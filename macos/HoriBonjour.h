// Copyright (C) 2026 hhhoratioxu. MIT; see LICENSE.
#pragma once
#include "../spotraop/libmdns/targets/include/mdnssd/mdnssd.h"
#ifdef __cplusplus
extern "C" {
#endif
bool hori_bonjour_query(struct in_addr bind, int seconds, mdns_callback_t* callback,
                        void* cookie, bool (*running)(void));
#ifdef __cplusplus
}
#endif
