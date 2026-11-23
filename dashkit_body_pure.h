#pragma once
#include <stdint.h>
#include <stddef.h>

// Independently adapted from DashKit Firmware (Softwired Technologies Ltd., MIT)
// dbc/bus_1_tesla_vehicle.dbc and main/controls/vehicle_control.{c,h}.
// No ESP-IDF driver code or TSL8 proprietary firmware is copied.
namespace dashkit_body {

static constexpr uint16_t UI_VEHICLE_CONTROL = 0x273u;
static constexpr uint16_t DI_GEAR = 0x118u;
static constexpr uint16_t DI_SPEED = 0x257u;
static constexpr uint16_t UI_VEHICLE_CONTROL2 = 0x3B3u;
static constexpr uint8_t GEAR_P = 1u;
static constexpr uint32_t CHASSIS_MAX_AGE_MS = 500u;
static constexpr uint32_t BODY_MAX_AGE_MS = 1200u;
static constexpr uint32_t BURST_SPACING_MS = 20u;
static constexpr uint8_t BURST_FRAMES = 10u;
static constexpr uint32_t REARM_SPACING_MS = 5000u;

enum Action : uint8_t {
  NONE = 0, OPEN_FRUNK = 1, OPEN_TRUNK = 2,
  FOLD_MIRRORS = 3, UNFOLD_MIRRORS = 4
};

static inline bool actionValid(uint8_t a) {
  return a == OPEN_FRUNK || a == OPEN_TRUNK ||
         a == FOLD_MIRRORS || a == UNFOLD_MIRRORS;
}
static inline const char *actionName(uint8_t a) {
  switch (a) {
    case OPEN_FRUNK: return "frunk";
    case OPEN_TRUNK: return "trunk";
    case FOLD_MIRRORS: return "mirror_fold";
    case UNFOLD_MIRRORS: return "mirror_unfold";
    default: return "none";
  }
}
static inline uint8_t actionParse(const char *s) {
  if (!s) return NONE;
  for (uint8_t a = OPEN_FRUNK; a <= UNFOLD_MIRRORS; a++) {
    const char *n = actionName(a);
    size_t i = 0;
    while (s[i] && n[i] && s[i] == n[i]) i++;
    if (!s[i] && !n[i]) return a;
  }
  return NONE;
}
static inline uint32_t littleBits(const uint8_t *d, uint8_t start, uint8_t bits) {
  if (!d || bits == 0 || bits > 16 || start + bits > 64) return 0;
  uint32_t v=0;
  for (uint8_t b=0;b<bits;b++) if ((d[(start+b)/8]>>((start+b)%8))&1) v|=1u<<b;
  return v;
}
static inline void putLittleBits(uint8_t *d, uint8_t start, uint8_t bits, uint8_t value) {
  if (!d || bits==0 || bits>8 || start+bits>64) return;
  for (uint8_t b=0;b<bits;b++) {
    const uint8_t mask=(uint8_t)(1u<<((start+b)%8));
    if ((value>>b)&1) d[(start+b)/8]|=mask;
    else d[(start+b)/8]&=(uint8_t)~mask;
  }
}
static inline uint8_t decodeGear(const uint8_t d[8]) {
  return (uint8_t)littleBits(d,21u,3u);
}
// DBC: DI_vehicleSpeed = raw * 0.08 - 40 kph; 500 = stopped.
// Use raw threshold <= 506 (0.48 kph) without floating point.
static inline bool speedStopped(const uint8_t d[8]) {
  const uint32_t raw=littleBits(d,12u,12u);
  return raw >= 500u && raw <= 506u;
}
static inline bool recent(uint32_t now, uint32_t at, uint32_t maxMs) {
  return at != 0 && (uint32_t)(now-at) <= maxMs;
}
static inline bool permitted(bool enabled,bool legacyY,bool chassisOnline,bool bodyOnline,
                              uint8_t gear,bool stopped,uint32_t now,
                              uint32_t gearMs,uint32_t speedMs,uint32_t bodyMs) {
  return enabled && legacyY && chassisOnline && bodyOnline &&
         gear==GEAR_P && stopped && recent(now,gearMs,CHASSIS_MAX_AGE_MS) &&
         recent(now,speedMs,CHASSIS_MAX_AGE_MS) &&
         recent(now,bodyMs,BODY_MAX_AGE_MS);
}
static inline bool compose(uint8_t action,uint8_t frame[8]) {
  if (!frame || !actionValid(action)) return false;
  // DashKit sends a zero-initialized UI_vehicleControl rather than echoing
  // unrelated stale stock fields from the instrument panel.
  for(uint8_t i=0;i<8;i++)frame[i]=0;
  // UI_remoteClosureRequest (bit 54..55): 1 rear / 2 front.
  // UI_mirrorFoldRequest (bit 24..25): 1 fold / 2 unfold.
  switch(action) {
    case OPEN_FRUNK: putLittleBits(frame,54u,2u,2u); break;
    case OPEN_TRUNK: putLittleBits(frame,54u,2u,1u); break;
    case FOLD_MIRRORS: putLittleBits(frame,24u,2u,1u); break;
    case UNFOLD_MIRRORS: putLittleBits(frame,24u,2u,2u); break;
    default: return false;
  }
  return true;
}
} // namespace dashkit_body
