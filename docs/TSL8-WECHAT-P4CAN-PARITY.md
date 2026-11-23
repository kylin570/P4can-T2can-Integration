# 「小王的特能拉」微信小程序 × P4CAN 功能逐项对照

核对日期：2026-11-23。P4CAN 对照版本：`f9d19a0`（在此版本基础上开发）。参照材料为用户上传的「小王的特能拉-小程序源码.zip」，检查了 `app.json`、`app-service.js` 中的页面模块、配置和设备通信调用。它包含 22 个普通页面和一个固件升级分包页面；包含微信编译生成的 JS 和 WXAPKG，不等同于项目作者公开、授权的原始仓库。

**版权边界**：未发现随包提供的 LICENSE，也未确认该小程序作者的开放许可。此项目仅分析可观察的功能和公开的页面行为，自行编写新的 P4CAN 代码；**不复制**小程序的源文件、图片、蓝牙指令实现或 TSL8 二进制固件进入 P4CAN 公开仓库。

**状态定义**：`已有`＝P4CAN 已存在同类功能入口（不能据此认定行为逐位等价）；`部分`＝有相近实现或仅覆盖部分；`新增`＝此次以 P4CAN 自己的接口独立增加；`缺少`＝当前没有对应完整功能；`设备专属`＝涉及 TSL8 专有硬件/协议/激活平台，不能只通过前端复制。

| 小程序页面 / 具体功能 | P4CAN 状态 | P4CAN 入口 / 实际差异 | 本次处理 |
|---|---|---|---|
| 首页：设备信息、车型、CAN 状态、常用功能 | 已有 | 主控制台 `/`；五种车型档案；3 CAN | 保留 |
| AP 辅助：DMS/注视提醒抑制 | 已有（策略不同） | Settings → Features → Disable Driver Monitoring | 保留，不重复 CAN 写入 |
| AP 辅助：限速提示抑制 ISA | 已有 | Settings → Features → ISA Suppression | 保留 |
| AP 辅助：禁用视觉限速 | 已有 | Settings → Features → Visual Speed Control | 保留 |
| AP 辅助：方向盘提醒抑制，滚轮/无感，随机间隔 | 部分 | Settings → Features → Nag Killer；Torque / TSL9 / Mode H / 滚轮；计时策略不同 | 保留单写入者 |
| AP 辅助：TLSSC 红绿灯 / 停车标志 | 已有 | Settings → Features → TLSSC | 保留 |
| AP 辅助：高速停用 / NOA 停用 TLSSC | 已有 | TLSSC → Block on Highways / Pause During NOA | 保留 |
| AP 辅助：绿灯自动起步 | 小程序标「暂不开放」；P4 不作为已实现 | 小程序自身注释未开放；不能认定实际可用 | 不虚构 |
| AP 辅助：打灯自动变道 | 已有 | Experimental → Auto Lane Change Enable；使用 P4 原有 0x293 控制管道 | 不重复 CAN 写入 |
| AP 辅助：青春版 EAP、一键 AP、恢复 AP | 部分 | Experimental → Continuous AP；方法与触发时序不同 | 待细化语义差异 |
| AP 辅助：手动远光 | 已有 | Features → Lighting Features | 保留 |
| AP 辅助：自动雨刮抑制 | 缺少同等专用功能 | 无对应面板 | 待硬件/协议确认 |
| AP 辅助：召唤、R79 相关控制 | 部分 | Experimental → R79 Mode；Summon Status 是监控页 | 不宣称逐位等效 |
| AP 辅助：变道激进度、ULC 盲点策略 | 部分 | Experimental → Lane Change | 参数映射不等价 |
| AP 辅助：FSD 可视化设置 | 部分 | Experimental → FSD Visualisation | 不等于 FSD 授权或车辆订阅 |
| AP 辅助：紧急车辆避让、车道线与导航 FSD 特效 | 缺少或仅有相近视觉设置 | 小程序部分功能要求 HW4 | 不伪造可用性 |
| 车辆控制：门锁、车窗、前/后备箱、后视镜 | 缺少 | 没有已确认的通用车身执行接口 | 不照搬不明 CAN 指令 |
| 车辆控制：主/副驾座椅调节、按摩、座椅记忆 | 缺少 | 不存在同等座椅子系统 | 需要车辆原始协议及执行反馈 |
| 车辆控制：副驾便利进出、空调烘干 | 缺少 | 不存在等价接口 | 需要新车型/状态机验证 |
| 车辆控制：方向盘滚轮手动指令 | 部分 | Nag/TSL9 存在模拟滚轮用法，但无小程序通用五向控制面板 | 不直接将通用指令接到车辆 |
| 仪表盘：速度、挡位、AP/NOA、方向盘信息 | 新增（精简版） | **`/gauge`**，读取现有 P4CAN 只读接口，不发送 CAN | **本次已实现** |
| 仪表个性化：深浅配色、更新周期 | 新增（部分） | `/gauge` → 外观/刷新周期，浏览器本地保存 | **本次已实现** |
| 仪表盘：SOC、里程、胎压、车门、摄像头视图 | 缺少 | 这些数据未由当前仪表接口可靠提供，不显示假数字 | 待解码及相应硬件 |
| 仪表盘：低速自动摄像头、转向灯全屏摄像头 | 缺少 | P4 没有同类摄像头投射链 | 需要显示端与事件协议 |
| 快捷指令：最高 127 条、按钮/动作绑定 | 缺少同等规则引擎 | P4 S3XY 最多 3 个已配对设备，按键手势动作映射；不是 127 条通用快捷指令 | 待新规则引擎与持久化设计 |
| 场景开关：动作条件与快捷指令组合 | 缺少 | P4 有功能配置，没有同等事件触发场景执行器 | 待统一事件词典/优先级 |
| 模块自动启停：条件规则 | 缺少 | 没有同等模块生命周期规则 | 待设计设备态约束 |
| 蓝牙按钮：扫描、配对、单/双击、绑定 | 部分 | Settings → Wireless / Devices / `/bluetooth`；S3XY 专用映射，协议不同 | 保留和继续扩展管理 |
| 蓝牙主机搜索外部按钮 | 部分 | P4 S3XY 蓝牙 Central 与 C6 Hosted BLE | 不复用 TSL8 UUID/指令 |
| RGB 氛围灯：6 区、渐变、联动、亮度配置 | 缺少 | 当前 P4 未实现独立 RGB 控制器 | 需要实际灯控硬件和协议 |
| TPMS：手机直读 Tesla `tsTPMS` 广播 | 缺少 | 原小程序由**手机微信蓝牙**扫描，不是从 CAN 获取 | 需要手机端 BLE 扫描组件；P4 不能冒充手机 |
| 电池状态：电芯、容量、温度与预热 | 部分 | Features → Battery Preheating，已有部分 BMS 温度/加热数据 | 仪表读取可用字段 |
| 数据调试：CAN 原始帧监控、导出 | 部分 | Settings → System → CAN Research Capture / Diagnostics / `/t2can` | 保留只读监控 |
| 数据调试：任意 CAN 帧发送 | 缺少公开的通用 TX 面板 | P4 的发送仅用于已定义功能的有限管道 | 不添加任意注入接口 |
| 安全带 CAN 采集实验 | 部分 | P4 的驾驶员监控/CAN 捕获能录帧，但不是同一实验向导 | 待独立向导 |
| 备份：设置、127 条快捷指令、蓝牙按钮全量备份与恢复 | 部分 + 新增 | **`/config-backup`** 为只读白名单字段快照及本地差异预览；非全量 NVS/TSL 备份 | **本次新增安全导出，不提供未验证自动恢复** |
| 模块配置：设备激活、服务器域名、拉黑、内测码 | 设备专属 | TSL8 原有账号/设备服务；不属于 P4 功能 | 不伪造接入 |
| 固件升级：小程序 OTA、Boot 更新、救砖 | 部分 | P4 `/update` 为 .bin 应用 OTA；`/recovery` 为双分区软件救援 | 不兼容 TSL8 .br / CH32 Boot |
| 常见维护：重启、恢复默认、服务模式 | 已有 | Settings → System → Diagnostics / Reset；`/recovery` | 保留 |

## 此次新增功能的真实能力

### `/gauge`：独立移动仪表

通过 GET `/api/snapshot?groups=home-fast&slow=1`、`/api/nag/stats`、`/api/preheat/stats`、`/api/t2can-monitor` 和 BLE 状态接口取值。仅在 `vehicleSpeedFresh=true` 时显示车速；仅在 BMS 标记有效时显示电池温度；显示三路 CAN 接收新鲜度。支持 2/5/10 秒刷新与本地浅色/深色主题。无 CAN 发送、车辆操作按钮或从缺失字段推算数字。

### `/config-backup`：安全参数快照

对明确白名单的若干现有 GET 接口提取布尔、数值和短文字参数，生成 `P4CAN_SETTINGS_SNAPSHOT_V1` JSON，本地下载；支持选择已保存 JSON 并预览差异。排除 Wi-Fi 密码、设备 VIN、BLE 配对信息和 CAN 原始帧。没有自动写回、没有 TSL8 格式导入，也不是对全量 NVS 的备份。

## 尚需专项开发的部分

1. **需要新增硬件/协议**：六区 RGB、座椅调节/按摩/记忆、门锁与车窗执行、手机原生 TPMS BLE 广播扫描、摄像头/显示投射。不能从小程序按钮名称直接推导 CAN 指令或假定现有 P4CAN 硬件支持。
2. **需要规则引擎、权限与持久化设计**：127 条快捷指令、场景联动、模块自动启停、全量配置恢复、车型差异化。应建立跨功能冲突处理，并由已有车辆控制管道单独负责执行，不能生成第二套 CAN TX。
3. **需避免误判**：小程序显示的“绿灯自动起步”明确标为“暂不开放”；部分 FSD 特性依赖车型、固件和车辆自身授权；不以界面显示作为已启用的证据。
4. **协议兼容**：小程序包含自己的 CH32/BLE 数据封包及 256 字节配置读写；P4CAN 为 ESP32-P4/C6、三路 TWAI 与完全不同的 NVS。两套固件和 Boot 不可直接合并或刷写。

## 文件证据定位（用户提供的压缩包）

- `app.json`：22 个普通页面、`packageFirmware` 升级分包。
- `app-service.js` → `pages/apHelper/apHelper.js`：AP、TLSSC、自动变道、辅助驾驶及部分标注“暂不开放”的设置。
- `pages/vehicleControl/vehicleControl.js`：门锁、座椅、车身与方向盘操作入口。
- `pages/gaugeMenu/gaugeMenu.js`：仪表、限速提示、摄像头触发等显示设置。
- `pages/shortcut/shortcut.js`、`pages/scene/scene.js`：快捷指令、规则绑定。
- `pages/bleHost/bleHost.js`、`pages/bleBtn/bleBtn.js`：外部按钮搜索配对及操作映射。
- `pages/rgb/rgb.js`：六区 RGB 设置与配置同步。
- `pages/tpms/tpms.js`：手机 BLE 监测 Tesla 胎压广播。
- `pages/backup/backup.js`：全量备份与恢复使用与 P4CAN 不同的数据格式。
- `pages/advancedSettings/advancedSettings.js`：漂移模式、前/后电机与模块维护入口。

> 该表仅代表已阅读的小程序功能/源代码行为与当前 P4CAN 源码覆盖情况，不证明 TSL8 所有功能都在每一辆车可用，也不代表已经实车移植。
