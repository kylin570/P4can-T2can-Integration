#pragma once
#include <stddef.h>
#include <stdint.h>

// Verify an exact key prefix before a command without early-byte mismatch
// exits. Secrets are never sent back in read/status BLE characteristics.
static inline bool p4BleAdminAuthorizePure(const char *input, size_t inputLength,
                                            const char *secret, size_t keyLength) {
  if (!input || !secret || keyLength < 8 || keyLength > 63 ||
      inputLength <= keyLength + 1 || inputLength > 100) return false;
  uint8_t mismatch = (uint8_t)(input[keyLength] ^ '|');
  for (size_t i = 0; i < keyLength; ++i)
    mismatch |= (uint8_t)(input[i] ^ secret[i]);
  return mismatch == 0;
}
