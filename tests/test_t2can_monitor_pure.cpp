#include "../t2can_monitor_pure.h"
#include <assert.h>
#include <stdint.h>

int main() {
  using namespace t2can_monitor;
  State s;
  View v = evaluate(s, 9000u, true, true);
  assert(!v.apFresh && !v.roadFresh && !v.apActive && !v.highwayKnown);

  for (uint8_t state = 0; state <= 8; ++state) {
    observeAp(s, state, 100u);
    v = evaluate(s, 100u, true, true);
    const bool active = state >= 3 && state <= 6;
    assert(v.apFresh && v.apActive == active);
    assert(v.noaActive == (state == 5u));
    assert(v.wouldPauseForNoa == (state == 5u));
  }
  observeAp(s, 5u, 100u);
  v = evaluate(s, 2100u, true, true);
  assert(v.apFresh && v.noaActive);
  v = evaluate(s, 2101u, true, true);
  assert(!v.apFresh && !v.apActive && !v.noaActive && !v.wouldPauseForNoa);

  observeAp(s, 5u, 200u);
  observeRoad(s, 2u, true, true, true, 205u);
  v = evaluate(s, 205u, true, true);
  assert(v.roadFresh && v.highwayKnown && v.highway);
  assert(v.wouldPauseForNoa && v.wouldPauseForHighway);
  v = evaluate(s, 205u, false, false);
  assert(v.noaActive && v.highway && !v.wouldPauseForNoa && !v.wouldPauseForHighway);

  observeRoad(s, 2u, false, true, true, 205u);
  v = evaluate(s, 206u, true, true);
  assert(v.roadFresh && !v.highwayKnown && !v.highway && !v.wouldPauseForHighway);
  observeRoad(s, 2u, true, false, false, 205u);
  v = evaluate(s, 206u, true, true);
  assert(v.highwayKnown && !v.highway);
  v = evaluate(s, 2206u, true, true);
  assert(!v.roadFresh && !v.highwayKnown && !v.highway);

  observeAp(s, 5u, UINT32_MAX - 40u);
  observeRoad(s, 4u, true, true, true, UINT32_MAX - 40u);
  v = evaluate(s, 10u, true, true);
  assert(v.apFresh && v.roadFresh && v.noaActive && v.highway);
  v = evaluate(s, 2100u, true, true);
  assert(!v.apFresh && !v.roadFresh && !v.apActive && !v.highway);
  assert(fresh(true, 0u, 2000u) && !fresh(true, 0u, 2001u));
  assert(!fresh(false, 100u, 101u));
  return 0;
}
