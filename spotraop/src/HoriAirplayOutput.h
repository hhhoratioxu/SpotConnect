// Copyright (C) 2026 hhhoratioxu. MIT; see LICENSE.
#pragma once
#ifdef __APPLE__
#ifdef __cplusplus
extern "C" {
#endif
// The ordinary RAOP handle remains the device identity. Only the tested HomePod
// model is routed through the separately licensed native AirPlay 2 sender.
bool hori_output_register(struct raopcl_s*, struct in_addr, const char* model);
bool hori_output_native(struct raopcl_s*);
bool hori_output_connect(struct raopcl_s*, struct in_addr, uint16_t, bool);
bool hori_output_disconnect(struct raopcl_s*);
bool hori_output_destroy(struct raopcl_s*);
bool hori_output_flush(struct raopcl_s*);
bool hori_output_keepalive(struct raopcl_s*);
bool hori_output_accept_frames(struct raopcl_s*);
bool hori_output_send_chunk(struct raopcl_s*, uint8_t*, int, uint64_t*);
raop_state_t hori_output_state(struct raopcl_s*);
bool hori_output_is_sane(struct raopcl_s*);
bool hori_output_is_connected(struct raopcl_s*);
bool hori_output_is_playing(struct raopcl_s*);
void hori_output_pause(struct raopcl_s*);
void hori_output_stop(struct raopcl_s*);
uint64_t hori_output_get_progress_ms(struct raopcl_s*);
bool hori_output_set_progress_ms(struct raopcl_s*, uint32_t, uint32_t);
bool hori_output_set_volume(struct raopcl_s*, float);
#ifdef __cplusplus
}
#endif
#ifndef HORI_OUTPUT_IMPLEMENTATION
#define raopcl_connect hori_output_connect
#define raopcl_disconnect hori_output_disconnect
#define raopcl_destroy hori_output_destroy
#define raopcl_flush hori_output_flush
#define raopcl_keepalive hori_output_keepalive
#define raopcl_accept_frames hori_output_accept_frames
#define raopcl_send_chunk hori_output_send_chunk
#define raopcl_state hori_output_state
#define raopcl_is_sane hori_output_is_sane
#define raopcl_is_connected hori_output_is_connected
#define raopcl_is_playing hori_output_is_playing
#define raopcl_pause hori_output_pause
#define raopcl_stop hori_output_stop
#define raopcl_get_progress_ms hori_output_get_progress_ms
#define raopcl_set_progress_ms hori_output_set_progress_ms
#define raopcl_set_volume hori_output_set_volume
#endif
#else
static inline bool hori_output_register(struct raopcl_s* p, struct in_addr ip, const char* model) {
    (void)p; (void)ip; (void)model; return false;
}
static inline bool hori_output_native(struct raopcl_s* p) { (void)p; return false; }
#endif
