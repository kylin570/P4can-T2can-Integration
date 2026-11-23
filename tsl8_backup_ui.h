#pragma once
// Export only explicitly whitelisted, existing P4CAN settings. This is a
// portable settings SNAPSHOT, not a raw NVS image, not a complete TSL backup.
// Wi-Fi credentials, BLE bond material, device IDs and VIN are never exported.
static const char P4CAN_BACKUP_HTML[] PROGMEM = R"BACKUP(<!doctype html><html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>P4CAN · 参数备份与对照</title><style>*{box-sizing:border-box}body{background:#0f172a;color:#e2e8f0;font:15px/1.65 system-ui,-apple-system,'Microsoft YaHei',sans-serif;margin:0}main{max-width:800px;margin:auto;padding:22px 16px}h1{font-size:24px;margin:0}.box{background:#1e293b;border:1px solid #405169;padding:16px;border-radius:13px;margin:17px 0}.muted{color:#a6b8d0}button,.file{background:#2563eb;border:0;border-radius:8px;color:white;padding:10px 13px;cursor:pointer;display:inline-block;margin:6px 5px 5px 0}input[type=file]{max-width:100%;display:block;margin:12px 0}pre{background:#111e30;color:#dbf0ff;border-radius:8px;padding:12px;white-space:pre-wrap;overflow-wrap:anywhere;max-height:450px;overflow:auto;font-size:12px}a{color:#93c5fd}h2{font-size:17px}.notice{border-left:3px solid #fbbf24;padding:8px 12px;color:#fed7aa;background:#35291b}code{font-size:12px}</style></head><body><main><h1>P4CAN 参数快照</h1><p class="muted">从设备已有只读接口导出一份便于保存和比对的参数摘要。参考小程序的“全量备份”需求，但**当前不支持自动还原车辆配置**。</p>
<div class="notice">与 TSL8 256 字节配置备份不是同一格式。本功能只备份经过筛选的字段，不含 Wi-Fi 密码、BLE 配对密钥、VIN、设备 ID 或 CAN 原始帧。导入文件仅在本浏览器预览，绝不会执行配置写入。</div>
<div class="box"><h2>1. 读取并导出</h2><button id="capture">重新读取当前设置</button><button id="download" disabled>下载 JSON 备份</button><div id="status" class="muted">尚未读取</div><pre id="out">—</pre></div>
<div class="box"><h2>2. 导入备份并对照</h2><p class="muted">选择本工具导出的 JSON 文件，检查版本及字段后，与最新一次读取的设备设置比较。不会触发任何 POST 请求。</p><input type="file" id="file" accept="application/json,.json"><pre id="diff">请选择文件</pre></div>
<p><a href="/">主界面</a> · <a href="/gauge">车辆仪表</a> · <a href="/recovery">恢复中心</a></p>
<script>
const $=x=>document.getElementById(x);const FORMAT='P4CAN_SETTINGS_SNAPSHOT_V1';
const schema=Object.freeze({
 nag:{url:'/api/nag/config',fields:['enabled','method','mode','pauseAtZeroSpeed','torqueRightScrollEnabled','torqueRightScrollIntervalSeconds','tsl9Sequence','tsl9InputMode']},
 driverMonitoring:{url:'/api/driver-monitoring/config',fields:['enabled','supported']},
 isa:{url:'/api/isa-suppression/config',fields:['enabled','supported']},
 continuousAp:{url:'/api/continuous-ap/config',fields:['enabled','method','supported']},
 lighting:{url:'/api/lighting/stats',fields:['enabled','apForcedHighBeamEnabled','brakeMode']},
 autoBlinker:{url:'/api/blinkA/stats',fields:['enabled','delayMs','noaWaitMs','cancelWaitMs']},
 autoLaneChange:{url:'/api/lab/auto-lane-change/stats',fields:['enabled','busMask','supported']},
 ulc:{url:'/api/ulc/stats',fields:['offHighwayMode','blindSpotMode','noConfirmEnabled','alcEnabled']},
 vision:{url:'/api/vision-control/stats',fields:['enabled','bus']},
 country:{url:'/api/country/stats',fields:['mode','mapMode']},
 batteryPreheat:{url:'/api/preheat/stats',fields:['enabled','targetC','supported']}
});
let snapshot=null;
function setText(id,text){$(id).textContent=text}
function filtered(source,fields){const out=Object.create(null);for(const key of fields){if(!Object.prototype.hasOwnProperty.call(source,key))continue;const v=source[key];if(typeof v==='string'){if(v.length<=80)out[key]=v}else if(typeof v==='boolean'||(typeof v==='number'&&Number.isFinite(v)))out[key]=v}return out}
async function readAll(){const sections={},unsupported=[];for(const [key,entry] of Object.entries(schema)){
 try{const r=await fetch(entry.url,{cache:'no-store',method:'GET'});if(!r.ok){unsupported.push(key+' (HTTP '+r.status+')');continue}const j=await r.json();if(!j||typeof j!=='object'||Array.isArray(j)){unsupported.push(key+' (invalid)');continue}sections[key]=filtered(j,entry.fields)}catch(e){unsupported.push(key+' (offline)')}}
 return{format:FORMAT,exportedAt:new Date().toISOString(),kind:'read-only sanitized feature subset',sections,unavailable:unsupported}}
function render(){if(!snapshot)return;setText('out',JSON.stringify(snapshot,null,2));setText('status','已读取 '+Object.keys(snapshot.sections).length+' 组，未提供 '+snapshot.unavailable.length+' 组。仅导出已确认字段。');$('download').disabled=false}
$('capture').onclick=async()=>{if($('capture').disabled)return;$('capture').disabled=true;setText('status','读取设备设置中…');try{snapshot=await readAll();render()}catch(e){setText('status','读取失败：'+e)}finally{$('capture').disabled=false}};
$('download').onclick=()=>{if(!snapshot)return;const content=JSON.stringify(snapshot,null,2);const url=URL.createObjectURL(new Blob([content],{type:'application/json;charset=utf-8'}));const a=document.createElement('a');a.href=url;a.download='p4can-settings-'+new Date().toISOString().slice(0,10)+'.json';a.click();setTimeout(()=>URL.revokeObjectURL(url),4000)};
$('file').onchange=async()=>{const file=$('file').files[0];if(!file)return;try{
 if(file.size>65536)throw Error('文件超过 64 KB');const parsed=JSON.parse(await file.text());if(!parsed||parsed.format!==FORMAT||!parsed.sections||typeof parsed.sections!=='object'||Array.isArray(parsed.sections))throw Error('不是本工具支持的参数快照');
 const diff=[];for(const [key,values] of Object.entries(parsed.sections)){if(!Object.prototype.hasOwnProperty.call(schema,key))continue;if(!values||typeof values!=='object'||Array.isArray(values))continue;
 const known=filtered(values,schema[key].fields);const current=snapshot?.sections?.[key];for(const [k,v] of Object.entries(known)){const now=current&&Object.prototype.hasOwnProperty.call(current,k)?current[k]:'未读取';if(now!==v)diff.push(key+'.'+k+': 备份='+JSON.stringify(v)+' · 设备='+JSON.stringify(now))}}
 setText('diff',(diff.length?diff.join('\n'):'没有发现不同或尚未读取设备')+'\n\n预览结束：未执行任何设备写入操作。');
 }catch(e){setText('diff','文件无效：'+e.message)}};
</script></main></body></html>)BACKUP";
