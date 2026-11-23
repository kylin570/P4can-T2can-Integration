#pragma once
#include <Preferences.h>
#include <esp_ota_ops.h>

// Software rescue is a separate boot policy, not an indestructible bootloader.
// The A/B partition table already has ota_0 and ota_1: retain the older image.
static bool p4RecoveryBootMode = false;
static volatile bool p4RecoveryButtonRequested = false;
static constexpr const char *P4_RECOVERY_NAMESPACE = "p4recovery";
static constexpr const char *P4_RECOVERY_BOOT_KEY = "next";

static bool p4RecoverySetNextBoot(bool value) {
  Preferences p;
  if (!p.begin(P4_RECOVERY_NAMESPACE, false)) return false;
  const bool ok = p.putBool(P4_RECOVERY_BOOT_KEY, value) == 1;
  p.end();
  return ok;
}
static bool p4RecoveryConsumeNextBoot() {
  Preferences p;
  if (!p.begin(P4_RECOVERY_NAMESPACE, false)) return false;
  const bool next = p.getBool(P4_RECOVERY_BOOT_KEY, false);
  // One-shot rescue is consumed before entering the web-only setup path.
  if (next) (void)p.putBool(P4_RECOVERY_BOOT_KEY, false);
  p.end();
  return next;
}
static bool p4RecoveryOtherImageValid() {
  const esp_partition_t *other = esp_ota_get_next_update_partition(nullptr);
  if (!other || other == esp_ota_get_running_partition()) return false;
  esp_app_desc_t desc = {};
  return esp_ota_get_partition_description(other, &desc) == ESP_OK;
}
static void p4RecoveryRequestFromButton() {
  __atomic_store_n(&p4RecoveryButtonRequested, true, __ATOMIC_RELEASE);
}
