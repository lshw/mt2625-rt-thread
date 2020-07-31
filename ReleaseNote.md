## v1.2.0

从该版本开始，使用标准的版本号命名风格，此版本为 v1.2.0。

v1.2.0 版本的核心修改是升级 MTK SDK 到最新版本，合入 MTK SDK 补丁。

更新内容如下所示：

- 增加 UART DEMO
- 增加 onenet OTA DEMO
- 增加 dump/read/write 串口配置的接口
- 释放 AT 串口，映射 AT 串口到 FINSH 串口，使用 `at_cmd_exec` 命令执行 AT 指令
- 增加 menuconfig 配置版本的功能
- 增加网络状态变化回调函数
- 修复 rtthread 软件定时器适配错误
- 更新 bootloader，增加 onenet OTA 升级功能
- 增加 scons 编译完成后，自动打包 onenet OTA 固件的功能
- 增加 onenet OTA 升级功能，支持差分升级
- 更新 onenet 接入 SDK
- 合并 MTK SDK 补丁
- 更新 MTK SDK 到最新版本

## v1.5.1-r1.1-openCPU

- 调整 SDK 结构，合并两个示例工程为 nbiot_demo 工程
- 增加 api.md 文档
- 增加 onenet 接入
- 增加 rt-thread mbedtls、webclient、mqtt 功能
- 更新 udp demo，增加接收功能
- 增加 ping、iperf 功能
- 增加 menuconfig 自定义 DNS 功能
- 增加 CESQ、CEREG、时间、ip、apn、derx、psm、sim 卡状态、信号强度等查询接口
- 增加软硬件版本和厂商 ID 查询接口
- 绑定 GKI 和 ULS 到 USB 虚拟串口 COM1 和 COM2
- 增加 powerkey 功能配置
- 优化 wdt 驱动及示例程序
- 增加 scons 编译成功后自动复制固件到 tools/download_config 目录的功能
- 增加 IMEI/SN/ICCID 等模组信息查询接口
- 修复 ifconfig 中 IMEI 错误的问题
- 增加 nvdm 固件，用于 RF 恢复

## v1.5.1-r1.0-openCPU

- 支持 RT-Thread OS 全功能
- 支持 scons 构建（支持 Windows、Linux 构建）
- 提供两个基本工程

    - nbiot_demo
    - peripheral_demo

- 支持 TCP/UDP/HTTP/TLS/DTLS/LWM2M 协议
- 支持 OneNET 接入
- 支持 socket 网络编程
- 支持 RT-Thread SAL 组件
- 支持低功耗

    IDLE 模式下 18ma；light sleep 模式下 6ma；deep sleep 模式下 50 ua。
    该功耗测试使用的是开发板，非独立模块的功耗。

- 支持二次开发
