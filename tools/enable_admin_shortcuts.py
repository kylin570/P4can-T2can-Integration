"""Idempotently expose the already-implemented administration features in the P4 dashboard."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def ensure(text, anchor, replacement):
    if replacement in text:
        return text
    if text.count(anchor) != 1:
        raise RuntimeError(f"Expected exactly one dashboard anchor, found {text.count(anchor)}")
    return text.replace(anchor, replacement, 1)


def main():
    p = ROOT / 'dashboard_source.html'
    raw = p.read_bytes()
    ending = '\r\n' if b'\r\n' in raw else '\n'
    source = raw.decode('utf-8').replace('\r\n', '\n')
    anchor = '<div class="listrow clickable" data-panel="panelWireless"><div><div class="listkey">Wireless</div><div class="listdesc" id="wirelessDesc">Bluetooth, S3XY reconnect and Wi-Fi</div></div><span class="chev">›</span></div>'
    source = ensure(source, anchor, anchor+'\n<div class="listrow"><div><div class="listkey"><a href="/bluetooth" style="color:inherit;text-decoration:underline">Bluetooth Administration / 手机蓝牙管理</a></div><div class="listdesc">BLE management, S3XY pairing and recovery gesture</div></div><span class="chev">›</span></div>')
    anchor = '<div class="listrow clickable" data-panel="panelDiag"><div><div class="listkey">Diagnostics / Recovery</div><div class="listdesc">Counters, boot capture and system logs</div></div><span class="chev">›</span></div>'
    if 'Software Recovery / 恢复中心' not in source:
        source = ensure(source, anchor, anchor+'\n<div class="listrow"><div><div class="listkey"><a href="/recovery" style="color:inherit;text-decoration:underline">Software Recovery / 恢复中心</a></div><div class="listdesc">Recovery boot, A/B application fallback, firmware uploader</div></div><span class="chev">›</span></div>')
    anchor = '<div class="listrow clickable" data-panel="panelOta"><div><div class="listkey">Firmware Update</div><div class="listdesc">Install an application .bin file</div></div><span class="chev">›</span></div>'
    source = ensure(source, anchor, anchor+'\n<div class="listrow"><div><div class="listkey"><a href="/update" style="color:inherit;text-decoration:underline">Authenticated Firmware Upload / 安全更新</a></div><div class="listdesc">Login: admin + your private Wi-Fi access point password</div></div><span class="chev">›</span></div>')
    # Independent compatibility tools - backed by existing read-only P4CAN APIs.
    anchor = '<div class="listrow clickable" data-panel="panelDiag"><div><div class="listkey">Diagnostics / Recovery</div><div class="listdesc">Counters, boot capture and system logs</div></div><span class="chev">›</span></div>'
    if 'Vehicle Gauge / 车辆仪表' not in source and 'Config Snapshot / 参数快照' not in source:
        source = ensure(source, anchor,
            anchor + '\n<div class="listrow"><div><div class="listkey"><a href="/gauge" style="color:inherit;text-decoration:underline">Vehicle Gauge / 车辆仪表</a></div><div class="listdesc">Live speed, AP/NOA and CAN status · read only</div></div><span class="chev">›</span></div>'
        + '\n<div class="listrow"><div><div class="listkey"><a href="/config-backup" style="color:inherit;text-decoration:underline">Config Snapshot / 参数快照</a></div><div class="listdesc">Export selected settings and compare a saved backup · no automatic restore</div></div><span class="chev">›</span></div>')
    anchor = '<div class="softNote" id="wifiPasswordHint">'
    source = ensure(source, anchor, '<div class="softNote"><a href="/bluetooth">Open Bluetooth Administration</a> · <a href="/recovery">Recovery Centre</a> · <a href="/update">Firmware OTA</a></div>\n'+anchor)
    anchor = "const actionOptions=[['none','None'],['noa_cancel','NOA Lane Change Cancel'],"
    source = ensure(source, anchor, "const actionOptions=[['none','None'],['recovery_portal','Recovery Portal (Wi-Fi only boot)'],['noa_cancel','NOA Lane Change Cancel'],")
    if raw != source.replace('\n', ending).encode('utf-8'):
        p.write_bytes(source.replace('\n', ending).encode('utf-8'))
    p = ROOT / 'bluetooth_admin_ui.h'
    src = p.read_text()
    anchor = '<div class="card"><h3>手机直接蓝牙管理</h3>'
    html = '<div class="card"><h3>S3XY 按钮打开恢复模式</h3><p>为已配对按钮分配 Recovery Portal 动作。触发后 P4CAN 重启进入只启动 Wi-Fi/OTA 的恢复模式。默认建议长按，不会自动打开手机浏览器。</p><label>按钮编号 <select id="slot"><option value="1">按钮 1</option><option value="2">按钮 2</option><option value="3">按钮 3</option></select></label> <label>手势 <select id="gesture"><option value="long">长按</option><option value="double">双击</option><option value="single">单击</option></select></label><button onclick="mapRecovery()">保存恢复手势</button></div>'
    if '<div class="card" id="s3xyMapCard">' not in src:
        src = ensure(src, anchor, html + anchor)
    anchor = 'async function send(url){'
    js = "async function mapRecovery(){try{const id=document.getElementById('slot').value;const gesture=document.getElementById('gesture').value;const url='/api/s3xy/device/action?'+new URLSearchParams({id,gesture,action:'recovery_portal'});const r=await fetch(url,{method:'POST'});document.getElementById('message').textContent=JSON.stringify(await r.json(),null,2);load()}catch(e){document.getElementById('message').textContent=String(e)}}\n"
    src = ensure(src, anchor, js + anchor)
    p.write_text(src)


if __name__ == '__main__':
    main()
