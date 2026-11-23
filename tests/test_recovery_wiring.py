from pathlib import Path
import unittest
P=Path(__file__).resolve().parents[1]
class RecoveryTests(unittest.TestCase):
 def test_software_rescue_and_ota_slots(self):
  sketch=(P/'P4-CAN-Universal-v3.35.3.ino').read_text()
  web=(P/'web_api.h').read_text()
  button=(P/'s3xy_ble.h').read_text()
  self.assertIn('vehicleProfileSetupMode = true;',sketch)
  self.assertIn('p4RecoveryConsumeNextBoot()',sketch)
  self.assertIn('esp_ota_set_boot_partition(other)',web)
  self.assertIn('p4RecoveryOtherImageValid()',web)
  for uri in ['/recovery','/api/recovery/status','/api/recovery/enter','/api/recovery/exit','/api/recovery/rollback']:
   self.assertIn('server.on("'+uri+'"',web)
  self.assertIn('S3XY_ACTION_RECOVERY_PORTAL = 17',button)
  self.assertIn('p4RecoveryRequestFromButton()',button)
  self.assertIn('recoveryButtonService()',web)
if __name__=='__main__':unittest.main()
