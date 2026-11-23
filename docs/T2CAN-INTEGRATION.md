# T-2CAN-inspired telemetry integration for P4CAN

This is a **read-only monitor integration**, not a combination of two stock firmwares.
The source base is `06066060606060/P4can-universal-unlock` (P4 firmware).
T-2CAN research reference: `emo666/t2can-lane-change`.

## What is actually integrated

- A small independently authored, pure C++ monitor records decoded AP/NOA and
  road-context status from P4CAN's **already validated receive handlers**.
- The original P4CAN TLSSC highway hysteresis remains the single source of truth;
  the monitor observes its result and does not run its own control policy.
- `GET /t2can` serves a standalone Chinese-language read-only monitor page.
- `GET /api/t2can-monitor` reports timestamp freshness, AP/NOA observation,
  highway classification, configured gate previews, and per-bus receive counters.
- No new CAN frames are constructed or transmitted by this integration.
- The existing P4CAN automatic lane change, TLSSC, nag, OTA and web settings
  remain untouched. There is no duplicated injector or separate control loop.

## What is *not* integrated or validated

- T-2CAN's MCP2515/TWAI board driver and its ESP32-S3 pin mapping are not
  compatible with the P4 ESP32 hardware and are not imported.
- No T-2CAN automatic-lane-change, TLSSC bit rewriting, camera override,
  right-scroll simulation, or Service Mode TX code is imported.
- No firmware image has been flashed to a car. This is not road-approved or
  live-CAN-tested. Traffic-light recognition / green-light launching is not
  demonstrated by the reference's status monitoring.
- The T-2CAN repository did not expose an explicit redistribution license when
  inspected on 2026-11-23; **none of its source files are copied** into this tree.

## Host tests

```sh
g++ -std=c++17 -Wall -Wextra -Werror -O2 tests/test_t2can_monitor_pure.cpp -o /tmp/t2can_monitor_test
/tmp/t2can_monitor_test
```

## Platform build

Follow the upstream P4CAN Arduino-ESP32 v3.35.3/ESP32-P4 instructions. A host
compiler alone cannot prove firmware compatibility. Hardware flashing is left
out intentionally until board revision, CAN topology and bench testing agree.

## API semantics

All values are observations, not proof of vehicle action. `wouldPauseForNoa`
and `wouldPauseForHighway` are *informational previews only*, not inputs to
CAN TX. Freshness expires after 2 seconds. Road context is displayed as
unknown when GPS match is missing or context data is stale.

## Browser display

After installing a **bench-verified** firmware build and connecting to the
board's AP, visit `http://<board-ip>/t2can`. The page polls
`/api/t2can-monitor` locally every 1.5 seconds and exposes no POST handlers.

## Automatic checks

The `.github/workflows/t2can-monitor-host-tests.yml` workflow runs C++ pure
logic tests and Python read-only wiring checks on push/PR. These checks do
**not** perform an ESP32-P4 board build or validate the firmware in a car.
