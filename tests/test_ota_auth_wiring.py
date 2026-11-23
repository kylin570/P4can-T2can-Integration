from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[1]
class OtaAuthWiringTests(unittest.TestCase):
 def test_ota_admin_gate_and_routes(self):
  s=(ROOT/'web_api.h').read_text(); ui=(ROOT/'firmware_update_ui.h').read_text()
  self.assertIn('server.on("/update", HTTP_GET, httpOtaPage)',s)
  self.assertIn('server.on("/update", HTTP_POST, httpOtaFinish, httpOtaUpload)',s)
  self.assertIn('if (up.status == UPLOAD_FILE_START && !otaRequestAuthenticated())',s)
  self.assertIn('if (!otaRequestAuthenticated()) return;',s)
  self.assertIn('if (!otaRequestAuthenticated()) { server.requestAuthentication(); return; }',s)
  self.assertIn('password != "12345678"',s)
  self.assertIn('otaUploadEnded && otaBytes > 0 && otaSuccess && !otaError',s)
  self.assertIn("new XMLHttpRequest()",ui)
  self.assertNotIn('<form method="POST"',ui)
if __name__=='__main__': unittest.main()
