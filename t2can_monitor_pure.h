#pragma once
#include <stdint.h>

// Independent, passive telemetry inspired by the T-2CAN monitoring use case.
// No third-party source code is reproduced. No TX methods or bus writes exist.
namespace t2can_monitor {
struct State {
  bool apSeen = false;
  uint8_t apState = 0xff;
  uint32_t apAtMs = 0;
  bool roadSeen = false;
  uint8_t roadClass = 0xff;
  bool gpsMatch = false;
  bool controlledAccess = false;
  bool highwayConfirmed = false;
  uint32_t roadAtMs = 0;
};
struct View {
  bool apFresh = false;
  bool roadFresh = false;
  bool apActive = false;
  bool noaActive = false;
  bool highwayKnown = false;
  bool highway = false;
  bool wouldPauseForNoa = false;
  bool wouldPauseForHighway = false;
};
inline void observeAp(State &s, uint8_t decodedState, uint32_t now) {
  s.apSeen = true;
  s.apState = decodedState;
  s.apAtMs = now;
}
inline void observeRoad(State &s, uint8_t roadClass, bool gpsMatch,
                       bool controlledAccess, bool highwayConfirmed,
                       uint32_t now) {
  s.roadSeen = true;
  s.roadClass = roadClass;
  s.gpsMatch = gpsMatch;
  s.controlledAccess = controlledAccess;
  s.highwayConfirmed = highwayConfirmed;
  s.roadAtMs = now;
}
inline bool fresh(bool seen, uint32_t atMs, uint32_t now,
                  uint32_t maxAgeMs = 2000u) {
  return seen && static_cast<uint32_t>(now - atMs) <= maxAgeMs;
}
inline View evaluate(const State &s, uint32_t now,
                     bool noaGateConfigured, bool highwayGateConfigured) {
  View v;
  v.apFresh = fresh(s.apSeen, s.apAtMs, now);
  v.roadFresh = fresh(s.roadSeen, s.roadAtMs, now);
  v.apActive = v.apFresh && s.apState >= 3u && s.apState <= 6u;
  v.noaActive = v.apFresh && s.apState == 5u;
  v.highwayKnown = v.roadFresh && s.gpsMatch;
  v.highway = v.highwayKnown && s.highwayConfirmed;
  v.wouldPauseForNoa = v.apActive && noaGateConfigured && v.noaActive;
  v.wouldPauseForHighway = v.apActive && highwayGateConfigured && v.highway;
  return v;
}
}  // namespace t2can_monitor
