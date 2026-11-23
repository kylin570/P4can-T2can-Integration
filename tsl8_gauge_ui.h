#pragma once
// P4CAN-native mobile vehicle display, inspired by the *feature category*
// of the uploaded mini-program. No third-party source/assets are copied.
// All vehicle fields come from the existing P4CAN read-only JSON endpoints.
static const char P4CAN_GAUGE_HTML[] PROGMEM = R"GAUGE(<!doctype html><html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1"><title>P4CAN · 车辆仪表</title>
<style>:root{color-scheme:dark}*{box-sizing:border-box}body{margin:0;font:15px/1.6 system-ui,-apple-system,'Microsoft YaHei',sans-serif;background:#09131f;color:#e8eef8}main{max-width:980px;margin:0 auto;padding:22px 14px}h1{font-size:24px;line-height:1.3;margin:0}h2{font-size:17px;margin:0 0 12px}.muted{color:#9ab0c9}a{color:#9fd4ff}header{display:flex;flex-wrap:wrap;align-items:center;justify-content:space-between;gap:12px}.tiny{font-size:12px}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:12px;margin:16px 0}.panel{background:#132338;border:1px solid #2b455d;border-radius:14px;padding:18px;min-width:0}.hero{text-align:center;min-height:226px;display:flex;flex-direction:column;align-items:center;justify-content:center}.speed{font-size:clamp(75px,16vw,124px);letter-spacing:-.07em;font-weight:600;line-height:1;min-height:1em}.unit{font-size:14px;color:#aac1dc;letter-spacing:.09em}.kv{display:grid;grid-template-columns:1fr auto;gap:9px;margin:0}.kv dt{color:#9ab0c9}.kv dd{margin:0;font-weight:600;text-align:right}.live{color:#61edb9}.stale{color:#fecf7e}.offline{color:#fca5a5}#net{margin:13px 0 0}select,button{border-radius:8px;padding:8px 10px;border:1px solid #4b647b;background:#1c354d;color:inherit}button{cursor:pointer}body.light{background:#f2f7fc;color:#182334}body.light .panel{background:#fff;border-color:#ceddeb}body.light .muted,body.light .kv dt,body.light .unit{color:#55677c}body.light select,body.light button{background:#eff5fa;color:#182334}.notice{border-left:3px solid #8db1c7;padding:7px 11px;margin:14px 0;color:#9ab0c9;font-size:13px}body.light .notice{color:#536d82}@media(min-width:720px){.hero{grid-column:span 2}}</style></head><body><main>
<header><div><h1>车辆状态仪表</h1><div class="muted tiny">独立开发 · 只读 · 基于 P4CAN 当前 CAN 接收数据</div></div><div><label>刷新 <select id="poll"><option value="2000">2 秒</option><option value="5000">5 秒</option><option value="10000">10 秒</option></select></label> <button id="theme" type="button">切换外观</button></div></header>
<div id="net" class="muted" role="status">正在连接设备…</div><div class="grid">
<section class="panel hero"><div class="speed" id="speed">—</div><div class="unit">km/h · 已接收的有效车速</div><div class="muted" id="speedStatus">无车速数据</div></section>
<section class="panel"><h2>驾驶状态</h2><dl class="kv"><dt>挡位</dt><dd id="gear">—</dd><dt>AP</dt><dd id="ap">—</dd><dt>NOA</dt><dd id="noa">—</dd><dt>DAS 原始状态</dt><dd id="das">—</dd><dt>方向盘转角</dt><dd id="steer">—</dd><dt>方向盘扭矩</dt><dd id="torque">—</dd></dl></section>
<section class="panel"><h2>CAN 总线</h2><dl class="kv"><dt>CAN 1 / BODY</dt><dd id="bus0">—</dd><dt>CAN 2 / CHASSIS</dt><dd id="bus1">—</dd><dt>CAN 3 / PARTY</dt><dd id="bus2">—</dd><dt>上次更新</dt><dd id="updated">—</dd></dl></section>
<section class="panel"><h2>电池与热管理</h2><dl class="kv"><dt>平均电池温度</dt><dd id="temp">—</dd><dt>最低 / 最高</dt><dd id="range">—</dd><dt>电池预热</dt><dd id="heat">—</dd><dt>加热功率（诊断）</dt><dd id="power">—</dd></dl><p class="muted tiny">仅显示车端可见且有效的 BMS/热管理数据，不推算剩余续航或 SOC。</p></section>
<section class="panel"><h2>模块连接</h2><dl class="kv"><dt>蓝牙管理</dt><dd id="ble">—</dd><dt>S3XY 已配对</dt><dd id="paired">—</dd><dt>S3XY 已连接</dt><dd id="connected">—</dd><dt>Wi-Fi</dt><dd id="wifi">本页已连接</dd></dl></section>
</div><div class="notice">与小程序原生仪表不同，本页面不伪造胎压、SOC、里程、门锁、车窗或摄像头数据。缺少相应 P4CAN 接收解析字段时明确显示「未提供」。不会发送车辆控制命令。</div><p><a href="/">主控制台</a> · <a href="/t2can">三路 CAN 监控</a> · <a href="/config-backup">参数快照</a></p>
<script>
const $=id=>document.getElementById(id);const settingsKey='p4can_gauge_ui_v1';let busy=false;let nextTimer=null;
function safeLoad(){try{const x=JSON.parse(localStorage.getItem(settingsKey)||'{}');return x&&typeof x==='object'?x:{}}catch{return {}}}
const prefs=safeLoad();$('poll').value=['2000','5000','10000'].includes(String(prefs.poll))?String(prefs.poll):'2000';if(prefs.light)document.body.classList.add('light');
function save(){try{localStorage.setItem(settingsKey,JSON.stringify({poll:Number($('poll').value),light:document.body.classList.contains('light')}))}catch{}}
$('theme').onclick=()=>{document.body.classList.toggle('light');save()};$('poll').onchange=()=>{save();schedule()};
function show(id,v,cls){const el=$(id);el.textContent=v===undefined||v===null||v===''?'—':String(v);el.className=cls||''}
const num=x=>typeof x==='number'&&Number.isFinite(x)?x:null;
function yes(x){return x===true?'开启':x===false?'关闭':'未知'}
async function get(url){const res=await fetch(url,{cache:'no-store'});if(!res.ok)throw Error(url+': HTTP '+res.status);return await res.json()}
async function maybe(url){try{return await get(url)}catch{return null}}
function busRender(i,b){if(!b){show('bus'+i,'未提供','stale');return}let n=(b.can||[]).find(x=>x.bus===i);if(!n){show('bus'+i,'未提供','stale');return}const age=num(n.lastRxAgeMs);const text=age===null?'未收到':age<=2000?'正在接收':'数据超时';show('bus'+i,text,age!==null&&age<=2000?'live':'stale')}
async function refresh(){if(busy)return;busy=true;try{
const [snap,nag,heat,ble,bus]=await Promise.all([get('/api/snapshot?groups=home-fast&slow=1'),maybe('/api/nag/stats'),maybe('/api/preheat/stats'),maybe('/api/ble-admin/status'),maybe('/api/t2can-monitor')]);
const f=snap.fast||{};const slow=snap.slow||{};const n=f.nag||{};const blink=f.blink||{};const gear=f.r79||{};const speed=num(nag?.vehicleSpeedKph);
show('speed',nag?.vehicleSpeedFresh===true&&speed!==null?speed.toFixed(0):'—');show('speedStatus',nag?.vehicleSpeedFresh===true?'车速有效':'车速超时 / 当前车型不支持',nag?.vehicleSpeedFresh===true?'live':'stale');
show('gear',gear.gearName==='UNKNOWN'?'未知':gear.gearName);show('ap',blink.dasStateValid?yes(n.apActive):'未知');show('noa',blink.dasStateValid?yes(blink.noaActive):'未知');show('das',blink.dasStateValid?blink.dasState:'—');
show('steer',nag&&nag.vehicleSpeedFresh===true&&num(nag.steeringDeg)!==null?Number(nag.steeringDeg).toFixed(1)+'°':'未提供');show('torque',num(n.torque)!==null?Number(n.torque).toFixed(2)+' Nm':'未提供');
for(let i=0;i<3;i++)busRender(i,bus);show('updated',new Date().toLocaleTimeString());
show('temp',heat?.packSeen===true&&heat?.ptAvgSeen===true&&num(heat.packTempAvgC)!==null?Number(heat.packTempAvgC).toFixed(1)+'°C':'未提供');
show('range',heat?.packSeen===true&&num(heat.packTempMinC)!==null&&num(heat.packTempMaxC)!==null?Number(heat.packTempMinC).toFixed(1)+' / '+Number(heat.packTempMaxC).toFixed(1)+'°C':'未提供');
show('heat',heat?yes(heat.enabled):'未提供');show('power',heat?.heatingSeen&&num(heat.statorHeatKw)!==null?Number(heat.statorHeatKw).toFixed(2)+' kW':'未提供');
show('ble',ble?yes(ble.bleServiceReady):'未提供');show('paired',slow.s3xy?.pairedCount??'未提供');show('connected',slow.s3xy?.connectedCount??'未提供');
show('net','设备在线 · '+new Date().toLocaleTimeString(),'live');
}catch(e){show('net','与设备连接失败：'+(e.message||e),'offline');show('updated','连接中断')}finally{busy=false;schedule()}}
function schedule(){if(nextTimer!==null)clearTimeout(nextTimer);nextTimer=setTimeout(refresh,Number($('poll').value)||2000)}
document.addEventListener('visibilitychange',()=>{if(document.hidden){if(nextTimer!==null)clearTimeout(nextTimer);nextTimer=null}else refresh()});
refresh();
</script></main></body></html>)GAUGE";
