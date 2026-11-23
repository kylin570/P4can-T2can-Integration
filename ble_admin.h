#pragma once
#include <BLEServer.h>
#include <BLEService.h>
#include <BLECharacteristic.h>
#include "ble_admin_auth_pure.h"

// ESP32-P4 Hosted BLE GATT maintenance. This service shares S3XY's BLE host,
// but never sends CAN frames or invokes CAN handlers from a BLE callback.
// Native NimBLE encryption + a private AP password on every write are required.
static constexpr const char *P4_BLE_ADMIN_SERVICE = "ae9d1100-286e-4e87-86cf-b3e52b77ff21";
static constexpr const char *P4_BLE_ADMIN_COMMAND = "ae9d1101-286e-4e87-86cf-b3e52b77ff21";
static constexpr const char *P4_BLE_ADMIN_REPLY   = "ae9d1102-286e-4e87-86cf-b3e52b77ff21";
static portMUX_TYPE bleAdminMux = portMUX_INITIALIZER_UNLOCKED;
static BLEServer *bleAdminServer = nullptr;
static BLECharacteristic *bleAdminReplyChar = nullptr;
static volatile bool bleAdminReady = false;
static volatile bool bleAdminConnected = false;
static volatile uint8_t bleAdminPending = 0;
static char bleAdminKey[64] = {};
static char bleAdminResult[120] = "READY";
static volatile uint32_t bleAdminDeniedCount = 0;
static volatile uint32_t bleAdminReplyRevision = 0;

enum BleAdminCommand : uint8_t { BLE_ADMIN_NONE = 0, BLE_ADMIN_WIFI_RESTART = 1,
                                  BLE_ADMIN_RESCUE = 2, BLE_ADMIN_S3XY_SCAN = 3 };
static void wifiApLoadConfig(String &ssid, String &password);
static bool wifiApActivate(const String &requestedSsid, const String &requestedPassword, bool allowFallback);

static void bleAdminReloadCredential() {
  String ssid, password;
  wifiApLoadConfig(ssid, password);
  char copy[64] = {};
  if (password.length() >= 8 && password.length() <= 63 && password != "12345678") {
    memcpy(copy, password.c_str(), password.length());
  }
  portENTER_CRITICAL(&bleAdminMux);
  memcpy(bleAdminKey, copy, sizeof(copy));
  portEXIT_CRITICAL(&bleAdminMux);
  memset(copy, 0, sizeof(copy));
}
static void bleAdminReply(const char *value) {
  portENTER_CRITICAL(&bleAdminMux);
  snprintf(bleAdminResult, sizeof(bleAdminResult), "%s", value ? value : "UNKNOWN");
  portEXIT_CRITICAL(&bleAdminMux);
  __atomic_add_fetch(&bleAdminReplyRevision, 1u, __ATOMIC_RELEASE);
}
// Constant-time comparison over the configured password; never echo or log it.
static bool bleAdminAuthorized(const String &payload, String &command) {
  char expected[64] = {};
  portENTER_CRITICAL(&bleAdminMux);
  memcpy(expected, bleAdminKey, sizeof(expected));
  portEXIT_CRITICAL(&bleAdminMux);
  const size_t keySize = strnlen(expected, sizeof(expected));
  const bool allowed = p4BleAdminAuthorizePure(payload.c_str(), payload.length(),
                                                 expected, keySize);
  memset(expected, 0, sizeof(expected));
  if (!allowed) return false;
  command = payload.substring(keySize + 1);
  return command.length() > 0 && command.length() < 40;
}
class P4BleAdminServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *server) override { (void)server; bleAdminConnected = true; }
  void onDisconnect(BLEServer *server) override { (void)server; bleAdminConnected = false; }
};
class P4BleAdminCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *chr) override {
    if (!chr) return;
    String cmd;
    if (!bleAdminAuthorized(chr->getValue(), cmd)) {
      __atomic_add_fetch(&bleAdminDeniedCount, 1u, __ATOMIC_RELAXED);
      bleAdminReply("DENIED: set a private WiFi password and authenticate");
      return;
    }
    if (cmd == "PING") { bleAdminReply("PONG"); return; }
    if (cmd == "STATUS") {
      bleAdminReply(p4RecoveryBootMode ? "RESCUE_RUNNING" : "P4CAN_RUNNING");
      return;
    }
    uint8_t code = BLE_ADMIN_NONE;
    if (cmd == "WIFI_RESTART") code = BLE_ADMIN_WIFI_RESTART;
    else if (cmd == "RESCUE") code = BLE_ADMIN_RESCUE;
    else if (cmd == "S3XY_SCAN") code = BLE_ADMIN_S3XY_SCAN;
    else { bleAdminReply("UNKNOWN_COMMAND"); return; }
    uint8_t expected = BLE_ADMIN_NONE;
    if (!__atomic_compare_exchange_n(&bleAdminPending, &expected, code, false,
                                     __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
      bleAdminReply("BUSY");
      return;
    }
    bleAdminReply("QUEUED");
  }
};
static P4BleAdminServerCallbacks bleAdminServerCallbacks;
static P4BleAdminCallbacks bleAdminCommandCallbacks;

static bool bleAdminStart() {
  if (bleAdminReady) return true;
  bleAdminReloadCredential();
  BLEServer *server = BLEDevice::createServer();
  if (!server) return false;
  server->setCallbacks(&bleAdminServerCallbacks);
  server->advertiseOnDisconnect(true);
  BLEService *service = server->createService(P4_BLE_ADMIN_SERVICE);
  if (!service) return false;
  BLECharacteristic *write = service->createCharacteristic(
      P4_BLE_ADMIN_COMMAND, BLECharacteristic::PROPERTY_WRITE |
                            BLECharacteristic::PROPERTY_WRITE_ENC);
  BLECharacteristic *reply = service->createCharacteristic(
      P4_BLE_ADMIN_REPLY, BLECharacteristic::PROPERTY_READ |
                          BLECharacteristic::PROPERTY_READ_ENC);
  if (!write || !reply) return false;
  write->setCallbacks(&bleAdminCommandCallbacks);
  reply->setValue("READY");
  if (!service->start()) return false;
  BLEAdvertising *adv = BLEDevice::getAdvertising();
  if (!adv) return false;
  adv->addServiceUUID(P4_BLE_ADMIN_SERVICE);
  adv->start();
  bleAdminReplyChar = reply;
  bleAdminServer = server;
  bleAdminReady = true;
  return true;
}
static void bleAdminShutdown() {
  bleAdminReady = false;
  bleAdminConnected = false;
  __atomic_store_n(&bleAdminPending, BLE_ADMIN_NONE, __ATOMIC_RELEASE);
  // BLEDevice owns and destroys the server during BLEDevice::deinit(false).
  bleAdminServer = nullptr;
  bleAdminReplyChar = nullptr;
}
static void bleAdminServiceTick() {
  static uint32_t publishedRevision = UINT32_MAX;
  const uint32_t revision = __atomic_load_n(&bleAdminReplyRevision, __ATOMIC_ACQUIRE);
  if (bleAdminReady && bleAdminReplyChar && publishedRevision != revision) {
    char reply[sizeof(bleAdminResult)];
    portENTER_CRITICAL(&bleAdminMux);
    memcpy(reply, bleAdminResult, sizeof(reply));
    portEXIT_CRITICAL(&bleAdminMux);
    bleAdminReplyChar->setValue(String(reply));
    publishedRevision = revision;
  }
  const uint8_t code = __atomic_exchange_n(&bleAdminPending, BLE_ADMIN_NONE, __ATOMIC_ACQ_REL);
  if (code == BLE_ADMIN_WIFI_RESTART) {
    String ssid, password;
    wifiApLoadConfig(ssid, password);
    const bool ok = wifiApActivate(ssid, password, true);
    bleAdminReply(ok ? "WIFI_RESTART_OK" : "WIFI_RESTART_FAILED");
  } else if (code == BLE_ADMIN_RESCUE) {
    bleAdminReply("RESCUE_REQUESTED");
    p4RecoveryRequestFromButton();
  } else if (code == BLE_ADMIN_S3XY_SCAN) {
    const bool ok = s3xyBluetoothMasterIsEnabled() && s3xyQueueCommand(S3XY_CMD_DISCOVERY_SCAN);
    bleAdminReply(ok ? "S3XY_SCAN_QUEUED" : "S3XY_UNAVAILABLE");
  }
}
