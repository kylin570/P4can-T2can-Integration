from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[1]
class BleAdminWiringTests(unittest.TestCase):
 def test_security_and_task_ownership(self):
  ble=(ROOT/'ble_admin.h').read_text(); s3=(ROOT/'s3xy_ble.h').read_text(); web=(ROOT/'web_api.h').read_text()
  self.assertIn('PROPERTY_WRITE_ENC',ble)
  self.assertIn('PROPERTY_READ_ENC',ble)
  self.assertIn('p4BleAdminAuthorizePure',ble)
  self.assertIn('password != "12345678"',ble)
  self.assertIn('bleAdminStart();',s3)
  self.assertIn('bleAdminShutdown();',s3)
  self.assertIn('bleAdminServiceTick();',web)
  self.assertIn('bleAdminStartRescueOnly()',web)
  self.assertIn('BLE_ADMIN_WIFI_RESET',ble)
  self.assertIn('WIFI_RESET_RECONNECT',ble)
  self.assertIn('wifiApPersistConfig(ssid, "12345678")',ble)
  self.assertIn('server.on("/bluetooth", HTTP_GET, httpBluetoothAdminPage)',web)
  self.assertIn('server.on("/api/ble-admin/status", HTTP_GET, httpBleAdminStatus)',web)
  self.assertIn('bleAdminReloadCredential();',web)
if __name__=='__main__': unittest.main()
