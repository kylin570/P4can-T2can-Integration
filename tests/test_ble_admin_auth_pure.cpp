#include "../ble_admin_auth_pure.h"
#include <assert.h>
#include <string.h>
int main() {
  const char *key = "a|secure-password-123";
  const size_t n = strlen(key);
  const char *valid = "a|secure-password-123|WIFI_RESTART";
  assert(p4BleAdminAuthorizePure(valid, strlen(valid), key, n));
  const char *valid2 = "a|secure-password-123|RESCUE";
  assert(p4BleAdminAuthorizePure(valid2, strlen(valid2), key, n));
  const char *wrong = "a|secure-password-124|WIFI_RESTART";
  assert(!p4BleAdminAuthorizePure(wrong, strlen(wrong), key, n));
  const char *shortOne = "a|secure-password-123|";
  assert(!p4BleAdminAuthorizePure(shortOne, strlen(shortOne), key, n));
  assert(!p4BleAdminAuthorizePure("", 0, key, n));
  assert(!p4BleAdminAuthorizePure(valid, strlen(valid), nullptr, n));
  assert(!p4BleAdminAuthorizePure(valid, strlen(valid), "short", 5));
  assert(!p4BleAdminAuthorizePure(valid, strlen(valid), key, 64));
  return 0;
}
