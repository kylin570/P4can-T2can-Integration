"""Static smoke check: the monitor must remain passive and fully routed."""
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[1]


class T2CanReadOnlyWiring(unittest.TestCase):
    def test_monitor_headers_and_handlers_are_connected(self):
        ino = (ROOT / "P4-CAN-Universal-v3.35.3.ino").read_text()
        logic = (ROOT / "vehicle_logic.h").read_text()
        web = (ROOT / "web_api.h").read_text()
        self.assertIn('#include "t2can_monitor_pure.h"', ino)
        self.assertIn('#include "t2can_monitor_ui.h"', ino)
        self.assertIn("t2can_monitor::observeAp(t2canMonitorState", logic)
        self.assertIn("t2can_monitor::observeRoad(t2canMonitorState", logic)
        self.assertIn('server.on("/t2can", HTTP_GET, httpT2CanMonitorPage)', web)
        self.assertIn('server.on("/api/t2can-monitor", HTTP_GET, httpT2CanMonitor)', web)
        self.assertNotRegex(web, r'server\.on\("/(?:t2can|api/t2can-monitor)",\s*HTTP_POST')

    def test_monitor_has_no_actuation_surface(self):
        headers = (ROOT / "t2can_monitor_pure.h").read_text() + (ROOT / "t2can_monitor_ui.h").read_text()
        web = (ROOT / "web_api.h").read_text()
        api = web.split("static void httpT2CanMonitor() {", 1)[1].split("static void httpPedalMapStats()", 1)[0]
        self.assertIn('"read-only"', api.replace('\\"', '"'))
        self.assertNotRegex(api, r"p4CanSend|canTxTwai|sendMessage|transmit_v2|fsdSettingsTransmit")
        self.assertNotRegex(headers, r"#include\s*<mcp2515\.h>|p4CanSend|canTxTwai|sendMessage\s*\(")
        self.assertNotIn('method="post"', headers.lower())


if __name__ == "__main__":
    unittest.main()
