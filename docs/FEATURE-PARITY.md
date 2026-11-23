# P4CAN × T-2CAN — Feature parity and integration audit

Audit date: 2026-11-23. Sources: P4CAN v3.35.3 and T-2CAN `emo666/t2can-lane-change` checked out under the integration workspace. Target: ESP32-P4, with P4CAN's existing three-CAN topology and ESP-Hosted C6 BLE/Wi-Fi stack.

| T-2CAN capability | P4CAN equivalent / location | Integration approach | Status |
|---|---|---|---|
| Two CAN buses, real-time diagnostics | `p4_can.h`, `can_runtime.h`, `/api/t2can-monitor` | Keep three native P4 CAN controllers, passive counters/status | Implemented (3 buses) |
| AP red-light state | `handle921`/DAS state, TLSSC / `handle1016` | Reuse existing DAS/TLSSC, mirror to read-only monitoring | Existing + monitor |
| Road/highway match and NOA block | `handleRoadContext238`, `tlsscHighwayHysteresis`, `/api/summon/tlssc-highway-gate`, `/api/summon/tlssc-noa-block` | One existing gate owner, no double injection | Existing |
| Indicator-initiated automatic lane-change enable | `uiAutoLaneChangeObserveAndInjectCanA/B`, `auto_lane_change_enable_pure.h` | T-2CAN explicitly credits the original P4CAN 0x293 logic; do not duplicate it | Existing |
| AP hands-on / "DND" monitoring | P4 `nag` and `driver-monitoring` modules | Use the existing P4 modes/gates; two policies are **not bit-for-bit identical** | Similar, not identical |
| Periodic right-scroll ±1 | `nagTorqueRightScrollEnabled`, `nagRightScrollPattern`, `nagTsl9RightPeriodicIntervalSeconds` | Keep P4 profile-gated right-scroll implementation | Existing (different scheduler) |
| Cabin camera bit43 DND behavior | P4 `dmsNag` / R79 DMS overlay | Use P4 isolated DMS feature; do not add a second 0x3FD writer | Existing mechanism, verify per model |
| Service Mode entry | `/api/service-mode/enter`, `/api/service-mode/restore` | Preserve P4 service-mode implementation | Existing |
| Wi-Fi panel, saved settings | `web_api.h`, P4 NVS | Retain P4's profile-scoped settings | Existing |
| Browser OTA firmware | `/update`, `/api/ota/status`, `Update` handler | Added authenticated GET page and protected POST/status | New; compile/test required |
| BLE button auto-reconnect / action mapping | `s3xy_ble.h` | Preserve S3XY central role | Existing |
| Manage device by phone BLE | `ble_admin.h`, `/bluetooth`, `tools/ble-manager.html` | Add encrypted GATT peripheral to shared C6 host; Wi-Fi secret as application credential | New; host/board testing required |
| Recovery after bad settings/update | `/recovery`, NVS `p4recovery`, ESP OTA A/B | Software rescue boot and select alternate OTA partition; no replacement bootloader | New; board validation required |
| Trigger recovery from vehicle-mounted input | S3XY mapping `recovery_portal` | Map to S3XY button gesture, schedule web-only boot, no CAN frame | New; requires mapping setup |

## OTA and boot boundaries

The 32MB `large_fat_32MB` Arduino-ESP32 partition scheme contains two OTA application slots (`ota_0`, `ota_1`) and OTA metadata, allowing fallback to the alternate valid image. Software rescue works only when an application can boot. A genuine "unbrickable" bootloader, ROM recovery button wiring, or automatic rollback on *pre-boot* firmware corruption is a separate board/bootloader project and has not been delivered by this code.

## BLE administration

- Set a private P4CAN Wi-Fi AP password in settings (not the default `12345678`).
- Enable the existing S3XY Bluetooth Master through `/bluetooth` once. The new service starts on the **same** ESP-Hosted NimBLE stack as the S3XY central; disabling the Master also shuts off BLE administration.
- A phone can connect with BLE after this; use `tools/ble-manager.html` from HTTPS/localhost, enter your private Wi-Fi password, then `PING`, `STATUS`, `WIFI_RESTART`, `S3XY_SCAN`, or `RESCUE`.
- GATT command characteristic has encrypted-write permission and enforces the private AP password on every request. BLE does **not** accept firmware blobs directly; use `/update` after restoring AP access.
- There is no supported way to force-launch a phone browser merely by pressing a Tesla/CAN button. A mapped S3XY long-press can request software rescue and make `/recovery` available after reboot; opening the page remains a phone action.

## Known concerns

- Existing P4 host test suite had failures before integration in the unmodified upstream version. The new host checks do not establish full vehicle behavior.
- New BLE peripheral and existing S3XY central concurrency must be tested on the actual P4/C6 board.
- Do not overwrite the original P4 CAN 0x293 or the shared 0x3FD overlay with a second T-2CAN writer; only one control pipeline should own those frames.
- Hosted-C6 BLE manager is not a second bootloader. If application and OTA slots are unusable, use board-specific ROM/USB recovery.
