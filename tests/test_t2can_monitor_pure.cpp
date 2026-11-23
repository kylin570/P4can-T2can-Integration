#include "../t2can_monitor_pure.h"
#include <assert.h>
#include <stdint.h>
int main() {
  using namespace t2can_monitor;
  State s;
  View v = evaluate(s, 9000u, true, true);
  assert(!v.apFresh && !v.roadFresh && !v.apActive);
  observeAp(s, 5u, 100u);
  v = evaluate(s, 100u, true, true);
  assert(v.apFresh && v.apActive && v.noaActive && v.wouldPauseForNoa);
  assert(!v.wouldPauseForHighway);
  observeRoad(s, 2u, true, true, true, 110u);
  v = evaluate(s, 110u, true, true);
  assert(v.highwayKnown && v.highway && v.wouldPauseForHighway);
  v = evaluate(s, 2111u, true, true);
  assert(!v.apFresh && !v.roadFresh && !v.apActive);
  observeAp(s, 3u, UINT32_MAX - 40u);
  v = evaluate(s, 10u, true, true);
  assert(v.apFresh && v.apActive && !v.noaActive);
  observeRoad(s, 4u, false, true, true, 11u);
  v = evaluate(s, 20u, true, true);
  assert(!v.highwayKnown && !v.highway && !v.wouldPauseForHighway);
  observeAp(s, 0u, 30u);
  v = evaluate(s, 31u, true, true);
  assert(!v.apActive && !v.noaActive);
  return 0;
}
