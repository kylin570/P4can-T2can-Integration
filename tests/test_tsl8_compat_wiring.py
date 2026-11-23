"""Fail-closed static tests for source-derived, no-transmit mini-program tools."""
from pathlib import Path
import re
import unittest
ROOT=Path(__file__).resolve().parents[1]
class MiniProgramTools(unittest.TestCase):
    def test_routes_are_get_only(self):
        sketch=(ROOT/'P4-CAN-Universal-v3.35.3.ino').read_text()
        web=(ROOT/'web_api.h').read_text()
        for label,handler in [('/gauge','httpTsl8GaugePage'),('/config-backup','httpTsl8BackupPage')]:
            self.assertIn(f'server.on("{label}", HTTP_GET, {handler})', web)
            self.assertNotIn(f'server.on("{label}", HTTP_POST',web)
        self.assertIn('#include "tsl8_gauge_ui.h"',sketch)
        self.assertIn('#include "tsl8_backup_ui.h"',sketch)
    def test_no_vehicle_commands_in_new_pages(self):
        gauge=(ROOT/'tsl8_gauge_ui.h').read_text()
        backup=(ROOT/'tsl8_backup_ui.h').read_text()
        for text in (gauge,backup):
            self.assertNotIn('HTTP_POST',text)
            self.assertNotIn("method:'POST'",text)
            self.assertNotIn('canTxTwai',text)
            self.assertNotIn('s3xyQueueCommand',text)
            self.assertNotIn('Update.begin',text)
        self.assertIn('vehicleSpeedFresh',gauge)
        self.assertIn('packSeen',gauge)
        self.assertIn('lastRxAgeMs',gauge)
        self.assertIn('P4CAN_SETTINGS_SNAPSHOT_V1',backup)
        self.assertIn('Object.prototype.hasOwnProperty.call',backup)
        self.assertIn('file.size>65536',backup)
        self.assertNotIn('wifiApLoadConfig',backup)
    def test_no_third_party_code_is_bundled(self):
        for path in ('tsl8_gauge_ui.h','tsl8_backup_ui.h'):
            s=(ROOT/path).read_text()
            self.assertNotIn('wx.writeBLECharacteristicValue',s)
            self.assertNotIn('CH32V20x',s)
if __name__=='__main__': unittest.main()
