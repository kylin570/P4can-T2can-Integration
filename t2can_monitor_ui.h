#pragma once
#include <Arduino.h>

// Standalone read-only diagnostic page. No forms, buttons or command endpoints.
static const char T2CAN_MONITOR_HTML[] PROGMEM = R"T2CAN(<!doctype html>
<html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>P4CAN · T-2CAN 监控</title>
<style>
*{box-sizing:border-box}body{background:#111827;color:#f1f5f9;font:15px/1.55 system-ui,-apple-system,'Microsoft YaHei',sans-serif;margin:0}main{max-width:790px;padding:26px 18px;margin:auto}h1{font-size:25px;margin:0 0 5px}p{color:#94a3b8;margin:5px 0 16px}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:12px}.card{background:#1f2937;padding:18px;border:1px solid #334155;border-radius:13px}.card h2{font-size:17px;margin:0 0 12px}dl{display:grid;grid-template-columns:1fr auto;gap:7px;margin:0}dt{color:#94a3b8}dd{margin:0;font-weight:600}.good{color:#4ade80}.warn{color:#fbbf24}.err{color:#fb7185}.muted{color:#94a3b8}.bar{display:flex;align-items:center;justify-content:space-between;gap:10px;flex-wrap:wrap;margin-bottom:18px}.mark{background:#064e3b;color:#a7f3d0;padding:4px 9px;border-radius:20px;font-size:12px}a{color:#93c5fd}small{color:#94a3b8}pre{white-space:pre-wrap;overflow-wrap:anywhere}
</style></head><body><main>
<div class="bar"><div><h1>P4CAN / T-2CAN 只读监控</h1><small>仅显示收到的 CAN 状态，不发送任何车辆指令</small></div><span class="mark">READ ONLY</span></div>
<p id="notice">正在获取监控数据…</p>
<div class="grid">
<section class="card"><h2>辅助驾驶 AP / NOA</h2><dl><dt>AP 数据</dt><dd id="apfresh">—</dd><dt>原始状态码</dt><dd id="apstate">—</dd><dt>AP 激活</dt><dd id="ap">—</dd><dt>NOA 激活</dt><dd id="noa">—</dd><dt>NOA 门控预览</dt><dd id="noag">—</dd></dl></section>
<section class="card"><h2>道路上下文</h2><dl><dt>道路数据</dt><dd id="roadfresh">—</dd><dt>GPS 道路匹配</dt><dd id="gps">—</dd><dt>高速状态</dt><dd id="highway">—</dd><dt>高速门控预览</dt><dd id="highwayg">—</dd></dl></section>
<section class="card"><h2>CAN 1</h2><dl><dt>接收帧累计</dt><dd id="bus0count">—</dd><dt>距上次接收</dt><dd id="bus0age">—</dd></dl></section>
<section class="card"><h2>CAN 2</h2><dl><dt>接收帧累计</dt><dd id="bus1count">—</dd><dt>距上次接收</dt><dd id="bus1age">—</dd></dl></section>
<section class="card"><h2>CAN 3</h2><dl><dt>接收帧累计</dt><dd id="bus2count">—</dd><dt>距上次接收</dt><dd id="bus2age">—</dd></dl></section>
</div>
<p><small>门控仅为现有配置的状态预览，不会触发自动变道、绿灯起步、滚轮模拟或任何 CAN 发送。状态超过 2 秒标记为过期。每 1.5 秒刷新一次。<a href="/">返回 P4CAN 主界面</a></small></p>
</main><script>
const $=id=>document.getElementById(id);
function val(id,t,cls){const e=$(id);e.textContent=t;e.className=cls||''}
function yes(x){return x?'是':'否'}
function fresh(id,x){val(id,x?'有效':'过期 / 未收到',x?'good':'warn')}
function gate(id,x,configured){val(id,!configured?'未配置':x?'触发暂停预览':'未触发',x?'warn':'muted')}
async function update(){try{
 const r=await fetch('/api/t2can-monitor',{cache:'no-store'});if(!r.ok)throw Error('HTTP '+r.status);
 const s=await r.json();if(s.mode!=='read-only'||s.canTxAdded!==false)throw Error('非只读模式，停止展示');
 fresh('apfresh',s.apFresh);val('apstate',s.apFresh?String(s.apState):'—');val('ap',s.apFresh?yes(s.apActive):'未知');val('noa',s.apFresh?yes(s.noaActive):'未知');gate('noag',s.wouldPauseForNoa,s.noaGateConfigured);
 fresh('roadfresh',s.roadFresh);val('gps',s.roadFresh?yes(s.gpsMatch):'未知');val('highway',s.highwayKnown?yes(s.highwayConfirmed):'未知');gate('highwayg',s.wouldPauseForHighway,s.highwayGateConfigured);
 for(let i=0;i<3;i++){const b=(s.can||[]).find(x=>x.bus===i);val('bus'+i+'count',b?String(b.rxCount):'未知');const age=b?.lastRxAgeMs;val('bus'+i+'age',typeof age==='number'?age+' ms':'未收到',age===null?'warn':age<=2000?'good':'warn')}
 val('notice','只读监控在线 · '+new Date().toLocaleTimeString(),'good');
}catch(e){val('notice','监控接口不可用：'+String(e.message||e),'err')}}
update();setInterval(update,1500);
</script></body></html>)T2CAN";
