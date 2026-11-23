'use strict';
const assert=require('node:assert/strict');
const fs=require('node:fs');
const vm=require('node:vm');
function script(path){const s=fs.readFileSync(path,'utf8');const m=s.match(/<script>([\s\S]*?)<\/script>/);assert(m,`script missing in ${path}`);return m[1]}
function makeDom(){
 const nodes=new Map();
 function create(id){return {id,value:id==='poll'?'2000':'',textContent:'',disabled:false,files:[],className:'',classList:{flags:new Set(),add(x){this.flags.add(x)},contains(x){return this.flags.has(x)},toggle(x){this.flags.has(x)?this.flags.delete(x):this.flags.add(x)}}}}
 return {nodes,getElementById(id){if(!nodes.has(id))nodes.set(id,create(id));return nodes.get(id)},createElement(){return {click(){},set href(x){},set download(x){}}},body:create('body'),addEventListener(){},hidden:false};
}
function makeContext(document, fetch){const saved=new Map();return vm.createContext({document,fetch,localStorage:{getItem:k=>saved.get(k)||null,setItem:(k,v)=>saved.set(k,v)},console,Date,JSON,Math,String,Number,Object,Array,Promise,Error,setTimeout:()=>1,clearTimeout:()=>{},URL:{createObjectURL:()=>'/local/file',revokeObjectURL:()=>{}},Blob:class{constructor(){}}});}
async function drain(){await new Promise(resolve=>setImmediate(resolve));}
async function gaugeTest(){
 const d=makeDom(),calls=[];
 let speedValid=true;
 const fetch=async (url,opt)=>{calls.push({url,opt});const obj=String(url).includes('/nag/stats')?{vehicleSpeedKph:56.7,vehicleSpeedFresh:speedValid,steeringDeg:1.7}:String(url).includes('/preheat/stats')?{packSeen:true,ptAvgSeen:true,packTempAvgC:22.1,packTempMinC:21.3,packTempMaxC:23.1,heatingSeen:true,statorHeatKw:1.3,enabled:true}:String(url).includes('/ble-admin/status')?{bleServiceReady:true}:String(url).includes('/t2can-monitor')?{can:[{bus:0,lastRxAgeMs:98},{bus:1,lastRxAgeMs:3400},{bus:2,lastRxAgeMs:null}]}:{fast:{nag:{apActive:true,torque:0.25},blink:{dasStateValid:true,noaActive:false,dasState:3},r79:{gearName:'D'}},slow:{s3xy:{pairedCount:2,connectedCount:1}}};return {ok:true,json:async()=>obj};};
 const ctx=makeContext(d,fetch);vm.runInContext(script('tsl8_gauge_ui.h'),ctx);await drain();
 assert.equal(d.getElementById('speed').textContent,'57');assert.equal(d.getElementById('gear').textContent,'D');assert.equal(d.getElementById('temp').textContent,'22.1°C');
 assert.equal(d.getElementById('bus0').textContent,'正在接收');assert.equal(d.getElementById('bus1').textContent,'数据超时');
 assert.equal(d.getElementById('paired').textContent,'2');assert(calls.every(x=>!x.opt||x.opt.method===undefined||x.opt.method==='GET'));
 speedValid=false;await vm.runInContext('refresh()',ctx);assert.equal(d.getElementById('speed').textContent,'—');
 console.log('PASS gauge derives freshness and real fields; no write methods');
}
async function backupTest(){
 const d=makeDom(),calls=[];
 const fetch=async (url,opts)=>{calls.push({url,opts});const obj=String(url).includes('/nag/config')?{enabled:true,method:1,mode:2,wifiPassword:'PRIVATE_SECRET',vin:'SECRET_ID'}:String(url).includes('/preheat/stats')?{enabled:false,targetC:30,packTempAvgC:25.5,vin:'SECRET_VIN'}:String(url).includes('/driver-monitoring/config')?{enabled:true,supported:true,bleKey:'NO'}:{};return {ok:!String(url).includes('/lighting/stats'),status:404,json:async()=>obj};};
 const ctx=makeContext(d,fetch);vm.runInContext(script('tsl8_backup_ui.h'),ctx);
 await d.getElementById('capture').onclick();
 const out=JSON.parse(d.getElementById('out').textContent);assert.equal(out.format,'P4CAN_SETTINGS_SNAPSHOT_V1');assert.equal(out.sections.nag.method,1);assert.equal(out.sections.batteryPreheat.targetC,30);
 assert(!JSON.stringify(out).includes('SECRET'));assert(!JSON.stringify(out).includes('25.5'));assert(calls.length>=8&&calls.every(x=>x.opts.method==='GET'));
 d.getElementById('file').files=[{size:100,text:async()=>JSON.stringify(out)}];await d.getElementById('file').onchange();assert(d.getElementById('diff').textContent.includes('未执行任何设备写入操作'));
 d.getElementById('file').files=[{size:70000,text:async()=>JSON.stringify(out)}];await d.getElementById('file').onchange();assert(d.getElementById('diff').textContent.includes('文件超过 64 KB'));
 console.log('PASS sanitized backup export and import preview, no writes');
}
(async()=>{await gaugeTest();await backupTest()})().catch(e=>{console.error(e);process.exitCode=1});
