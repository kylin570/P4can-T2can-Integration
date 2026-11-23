# DashKit Firmware → P4CAN Model Y Legacy adaptation

Source project: <https://github.com/softwiredtech/dashkit-firmware> (Softwired Technologies Ltd., MIT license). Original implementation inspected: `main/controls/vehicle_control.c`, `.h`, `dbc/bus_1_tesla_vehicle.dbc` and `main/automation/automation_manager.c`. This integration is independently adapted to P4CAN's Arduino ESP32-P4 triple-TWAI architecture. No binary firmware from TSL8/CH32V20x and no DashPilot app are installed.

## Why DashKit, not the other candidates?

- **DashKit**: MIT source with actual vehicle CAN TX, decoded DBC bit positions, state-aware vehicle control and FreeRTOS task design. The least invasive path for P4CAN native TWAI.
- **S3XYCommander**: MIT Swift client lists many seat/window/door actions, but those call a separate S3XY Commander firmware; NOT the raw vehicle CAN packet implementation.
- **Tesla vehicle-command**: official authorization-based BLE commands, needs separate pairing/session cryptography, not a native P4CAN CAN module and not the seat-massage CAN protocol.
- **Tesla CAN signal database / model3dbc**: valuable signal decoding, not complete actuator implementations.
- **ArduinoTeslaSeat**: physically drives removed Model X seat motors via relays, incompatible with onboard Model Y OEM seat modules.

## Current port (`/body-controls`)

Feature | DBC message | Signal | Current implementation
---|---|---|---
Open front trunk | `UI_vehicleControl` 0x273 | `UI_remoteClosureRequest`, 54|2 little-endian, 2 | Implemented as parked-only manual request
Open rear trunk | 0x273 | Same field, value 1 | Implemented as parked-only manual request
Mirror fold | 0x273 | `UI_mirrorFoldRequest`, 24|2, value 1 | Implemented as parked-only manual request
Mirror unfold | 0x273 | Same field, value 2 | Implemented as parked-only manual request
Door unlatch | `VCSEC_requests` 0x1F9 | Per-door closure and reason | Not ported; requires model/vehicle feedback verification
Glovebox | `UI_vehicleControl2` 0x3B3 | `UI_gloveboxRequest` | Not yet ported (RMW/pulse arbitration)
Lock/unlock/windows | Multiple signals | Depends on firmware | Not yet ported
Seat adjust/memory/massage | No DashKit CAN implementation | Not available | Not yet ported
127 shortcut rules/scenes | DashKit automation pattern | DashKit has a few fixed automations | Not yet implemented as 127-rule engine

### Enabling and route

1. Select **Model Y Legacy** in P4CAN vehicle profile; this port rejects all other models.
2. Choose a **private Wi-Fi AP password** in Wireless settings. HTTP Basic authentication uses username `admin` and that password.
3. Open `http://192.168.4.1/body-controls` (or the gateway IP shown by your device) and opt in using **Enable module**. Default is disabled in NVS on first install.
4. Current body control uses physical P4CAN **CAN 1 (BODY, route A)**, while parked validation uses **CAN 2 (CHASSIS, route B)**.
5. For each operation, P4CAN must see **fresh stock 0x273** on BODY, **fresh 0x118 gear == P** and **fresh 0x257 speed ≤0.48 km/h** on CHASSIS; all three physical controllers must be running, the CAN admin barrier open, CAN USB not passive, and the device not in rescue or setup mode. Missing data means **no transmit**.
6. The app sends 10 bounded 0x273 requests at 20 ms nominal intervals, with a 5-second rearm time; stops immediately if a gate expires or TX fails. The API reports **CAN driver acceptance**, NOT that a vehicle actually opened/closed anything. A separate live vehicle state comparison is still required.

### Boundaries and follow-up work

- DashKit confirms its door-unlatch technique on **2023 Model Y**; this does not by itself verify the P4CAN X179 wiring, 2022 Model Y firmware, or future OTA versions.
- Only actual P4 CAN RX logs will confirm whether 0x273, 0x118 and 0x257 are present on the expected buses; no hardcoded reroute or fallback to another bus.
- P4CAN's existing AP, lane-change, TLSSC, NAG and other control writers are untouched; use P4CAN's native A TX gate and single CAN driver. Do **not** merge a second DashKit CAN stack or copy over P4CAN firmware bootloader.
- Door/seat/window control additions must have source-grounded semantics and explicit confirmation; do not manufacture CAN payloads from S3XYCommander high-level action names.
- This is currently **source + host/build tested**, not verified on the user's vehicle. OTA application artifact name remains the two `prev3`/`postv3` images and must match the ESP32-P4 chip revision.
