from pathlib import Path
import gzip
import re
import unittest
ROOT=Path(__file__).resolve().parents[1]
class AdminEntries(unittest.TestCase):
 def test_source_links(self):
  source=(ROOT/'dashboard_source.html').read_text()
  for route in ('/bluetooth','/recovery','/update'):
   self.assertIn('href="'+route+'"',source)
  self.assertIn("['recovery_portal','Recovery Portal (Wi-Fi only boot)']",source)
  bluetooth=(ROOT/'bluetooth_admin_ui.h').read_text()
  self.assertIn('action:\'recovery_portal\'',bluetooth)
  self.assertIn('gesture',bluetooth)
 def test_gzip_header_unchanged_format(self):
  header=(ROOT/'index_html.h').read_text()
  data=bytes(int(x,16) for x in re.findall(r'0x([0-9A-F]{2})',header))
  self.assertEqual(gzip.decompress(data),(ROOT/'dashboard_source.html').read_bytes())
if __name__=='__main__':unittest.main()
