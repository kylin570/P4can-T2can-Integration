from pathlib import Path
import unittest
P=Path(__file__).resolve().parents[1]
class WiringTests(unittest.TestCase):
 def test_can_routes_and_isolation(self):
  sketch=(P/'P4-CAN-Universal-v3.35.3.ino').read_text()
  can=(P/'can_runtime.h').read_text()
  web=(P/'web_api.h').read_text()
  runtime=(P/'dashkit_body_runtime.h').read_text()
  for header in ['dashkit_body_runtime.h','dashkit_body_ui.h']:
   self.assertIn('#include "'+header+'"',sketch)
  self.assertIn('dashkitBodyObserveBody(partyId, rxf.can_dlc, rxf.data)',can)
  self.assertIn('dashkitBodyObserveChassis(f.identifier, f.data_length_code, f.data)',can)
  for path in ['/body-controls','/api/dashkit-body/status','/api/dashkit-body/enable','/api/dashkit-body/action']:
   self.assertIn('server.on("'+path+'"',web)
  self.assertIn('dashkitBodyTick();',web)
  self.assertIn('dashkitBodyLoad();',web)
  self.assertIn('canTxNativeASend(&msg,canTxEpochSnapshot()',runtime)
  self.assertIn('VEHICLE_MODEL_Y_LEGACY',runtime)
  self.assertIn('!p4RecoveryBootMode',runtime)
  self.assertIn('!canMaintenanceActive()',runtime)
  self.assertIn('!canUsbPassive()',runtime)
  self.assertIn('!canTxAdministrativeHold',runtime)
  self.assertIn('pref.getBool("enabled", false)',runtime)
  self.assertIn('otaRequestAuthenticated()',web)
 def test_no_seat_unlock_or_unrestricted_tx(self):
  s=(P/'dashkit_body_runtime.h').read_text()
  self.assertNotIn('driverMoveSeat',s)
  self.assertNotIn('arbitrary',s.lower())
  self.assertNotIn('p4CanSendFrame(&msg)',s)
if __name__=='__main__':unittest.main()
