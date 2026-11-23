#pragma once
#include "dashkit_body_pure.h"
#include <Preferences.h>

// P4CAN adaptation of DashKit's parked-only UI_vehicleControl actions.
// Source provenance, DBC bit positions and license: docs/DASHKIT-PORT.md.
// Does NOT include seat movement, door unlocking, windows or remote controls.
static portMUX_TYPE dashkitBodyMux = portMUX_INITIALIZER_UNLOCKED;
struct DashkitBodyObserved {
  uint8_t gear = 0;
  bool stopped = false;
  uint32_t gearMs = 0;
  uint32_t speedMs = 0;
  uint32_t bodyMs = 0;
};
static DashkitBodyObserved dashkitBodyObserved = {};
static bool dashkitBodyEnabled = false;        // persist only on explicit admin choice
static uint8_t dashkitBodyPending = dashkit_body::NONE;
static uint8_t dashkitBodyFramesLeft = 0;
static uint32_t dashkitBodyLastRequestMs = 0;
static uint32_t dashkitBodyNextFrameMs = 0;
static uint32_t dashkitBodyTxAccepted = 0;
static uint32_t dashkitBodyTxFailed = 0;
static const char *dashkitBodyLastResult = "never_requested";

static void dashkitBodyObserveBody(uint32_t canId, uint8_t dlc, const uint8_t *data) {
  if (activeVehicleProfile != VEHICLE_MODEL_Y_LEGACY ||
      !activeProfileBodyControlsSupported() || !data || dlc != 8 ||
      canId != dashkit_body::UI_VEHICLE_CONTROL) return;
  const uint32_t now = (uint32_t)millis();
  portENTER_CRITICAL(&dashkitBodyMux);
  dashkitBodyObserved.bodyMs = now;
  portEXIT_CRITICAL(&dashkitBodyMux);
}
static void dashkitBodyObserveChassis(uint32_t canId, uint8_t dlc, const uint8_t *data) {
  if (activeVehicleProfile != VEHICLE_MODEL_Y_LEGACY ||
      !activeCanBIsChassis() || !data || dlc != 8) return;
  const uint32_t now = (uint32_t)millis();
  if (canId == dashkit_body::DI_GEAR) {
    const uint8_t gear = dashkit_body::decodeGear(data);
    portENTER_CRITICAL(&dashkitBodyMux);
    dashkitBodyObserved.gear = gear;
    dashkitBodyObserved.gearMs = now;
    portEXIT_CRITICAL(&dashkitBodyMux);
  } else if (canId == dashkit_body::DI_SPEED) {
    const bool stopped = dashkit_body::speedStopped(data);
    portENTER_CRITICAL(&dashkitBodyMux);
    dashkitBodyObserved.stopped = stopped;
    dashkitBodyObserved.speedMs = now;
    portEXIT_CRITICAL(&dashkitBodyMux);
  }
}
static DashkitBodyObserved dashkitBodyReadObserved() {
  DashkitBodyObserved s;
  portENTER_CRITICAL(&dashkitBodyMux);
  s=dashkitBodyObserved;
  portEXIT_CRITICAL(&dashkitBodyMux);
  return s;
}
static bool dashkitBodyPermitNow(const DashkitBodyObserved &s, uint32_t now) {
  const bool profile = activeVehicleProfile == VEHICLE_MODEL_Y_LEGACY &&
      activeCanAIsBody() && activeCanBIsChassis() && activeProfileBodyControlsSupported();
  const bool transport = p4CanAllRunning() && p4CanOnline[p4CanBusA()] &&
      p4CanOnline[p4CanBusB()] && !p4CanPassive[p4CanBusA()] &&
      !canMaintenanceActive() && !canUsbPassive() && !vehicleProfileSetupMode &&
      !p4RecoveryBootMode && !canTxAdministrativeHold;
  return dashkit_body::permitted(dashkitBodyEnabled, profile,transport,transport,
           s.gear,s.stopped,now,s.gearMs,s.speedMs,s.bodyMs);
}
static void dashkitBodyLoad() {
  Preferences pref;
  if (pref.begin("dashkitbody", true)) {
    dashkitBodyEnabled = pref.getBool("enabled", false);
    pref.end();
  } else dashkitBodyEnabled=false;
}
static bool dashkitBodySetEnabled(bool value) {
  // Intentional opt-in remains restricted to legacy Model Y.
  if (value && (activeVehicleProfile != VEHICLE_MODEL_Y_LEGACY ||
                !activeCanAIsBody() || !activeCanBIsChassis())) return false;
  Preferences pref;
  if (!pref.begin("dashkitbody", false)) return false;
  const bool saved=pref.putBool("enabled",value)==1;
  pref.end();
  if (!saved) return false;
  dashkitBodyEnabled=value;
  if (!value) {
    dashkitBodyPending=dashkit_body::NONE;
    dashkitBodyFramesLeft=0;
    dashkitBodyLastResult="disabled";
  }
  return true;
}
static bool dashkitBodySchedule(uint8_t action) {
  if (!dashkit_body::actionValid(action)) {dashkitBodyLastResult="invalid_action";return false;}
  if (dashkitBodyFramesLeft!=0) {dashkitBodyLastResult="busy";return false;}
  const uint32_t now=(uint32_t)millis();
  if (dashkitBodyLastRequestMs &&
       (uint32_t)(now-dashkitBodyLastRequestMs) < dashkit_body::REARM_SPACING_MS) {
    dashkitBodyLastResult="cooldown";return false;
  }
  if (!dashkitBodyPermitNow(dashkitBodyReadObserved(),now)) {
    dashkitBodyLastResult="parked_freshness_gate";return false;
  }
  dashkitBodyPending=action;
  dashkitBodyFramesLeft=dashkit_body::BURST_FRAMES;
  dashkitBodyNextFrameMs=now;
  dashkitBodyLastRequestMs=now;
  dashkitBodyLastResult="queued_not_vehicle_confirmed";
  return true;
}
static void dashkitBodyTick() {
  if (!dashkitBodyFramesLeft) return;
  const uint32_t now=(uint32_t)millis();
  if ((int32_t)(now-dashkitBodyNextFrameMs)<0) return;
  if (!dashkitBodyPermitNow(dashkitBodyReadObserved(),now)) {
    dashkitBodyFramesLeft=0;
    dashkitBodyPending=dashkit_body::NONE;
    dashkitBodyLastResult="aborted_freshness_gate";
    return;
  }
  uint8_t bytes[8] = {};
  if (!dashkit_body::compose(dashkitBodyPending,bytes)) {
    dashkitBodyFramesLeft=0;
    dashkitBodyPending=dashkit_body::NONE;
    dashkitBodyLastResult="invalid_payload";
    return;
  }
  struct CanFrame msg={};
  msg.bus=p4CanBusA();
  msg.can_id=dashkit_body::UI_VEHICLE_CONTROL;
  msg.can_dlc=8;
  memcpy(msg.data,bytes,8);
  esp_err_t err=ESP_FAIL;
  NativeATxResultReason reason=NATIVE_A_TX_INVALID_MSG;
  const bool admitted=canTxNativeASend(&msg,canTxEpochSnapshot(),err,&reason);
  if (!admitted || err!=ESP_OK) {
    dashkitBodyTxFailed++;
    dashkitBodyFramesLeft=0;
    dashkitBodyPending=dashkit_body::NONE;
    dashkitBodyLastResult="tx_rejected_or_failed";
    return;
  }
  dashkitBodyTxAccepted++;
  if (--dashkitBodyFramesLeft == 0) {
    dashkitBodyPending=dashkit_body::NONE;
    dashkitBodyLastResult="driver_tx_accepted_not_vehicle_confirmed";
  } else dashkitBodyNextFrameMs=now+dashkit_body::BURST_SPACING_MS;
}
