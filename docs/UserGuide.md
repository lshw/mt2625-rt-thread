# MT2625 SDK For RT-Thread 用户指南

本软件包是 MT2625 v1.2.0 版本 SDK 在 RT-Thread 操作系统上的移植实现。旨在让用户利用 RT-Thread 物联网生态中丰富组件和软件包快速实现项目需求。

本软件包提供了一个 `nbiot_demo`，开发者可以基于该工程完成自定义开发。

## nbiot_demo 工程

`nbiot_demo` 工程的 `application/main.c` 中完成了对 NB-IOT 网络的初始化工作，并关闭了自动睡眠功能。

你可以在该工程使用 RT-Thread 系统的所有组件。

为了方便测试，在 nbiot_demo 工程下提供了一些 demo 程序，位于 `project/nbiot_demo/samples` 目录下。这里的 demo 程序可以在 RT-Thread finsh 串口中使用命令进行快速测试。

示例程序包括 ADC、GPIO、GPT、PWM、WDT、SPI LCD、DUP、TCP、ONENET 等，文件列表如下：

```
samples
  +-- demo_gpio.c
  +-- demo_adc.c
  +-- demo_pwm.c
  +-- demo_wdt.c
  +-- demo_gpt.c
  +-- demo_lcd.c
  +-- lcd_st7789_spi.c
  +-- demo_tcp.c
  +-- demo_udp.c
  +-- demo_onenet.c
  +-- nb_onenet_callback.c
```

每一个测试 demo 都有对应 finsh 命令，使用 finsh 命令启动测试，测试命令以 `demo_` 开头，如下所示：

```
demo_gpio        - gpio driver sample
demo_adc         - demo_adc <adc num>
demo_pwm         - demo_pwm <pwm device num>
demo_wdt         - watchdog device test
demo_gpt         - hwtimer test
demo_lcd         - spi lcd test
demo_tcp         - nbiot tcp test
demo_udp         - nbiot udp test
demo_onenet      - nbiot onenet test
```

另外，RTC 可以使用 finsh 命令测试，参考[RT-Thread RTC 使用文档](https://www.rt-thread.org/document/site/programming-manual/device/rtc/rtc/#finsh)。

IIC 和 SPI 接口的传感器应用可以使用 RT-Thread 提供的[传感器框架](https://www.rt-thread.org/document/site/programming-manual/device/sensor/sensor/)。


## 编译

```
$ cd project/nbiot_demo
$ scons
```

正确编译完成后，提示如下：

```
LINK rt-thread.elf
arm-none-eabi-objcopy -O binary rt-thread.elf rtthread.bin
arm-none-eabi-size rt-thread.elf
   text	   data	    bss	    dec	    hex	filename
1936419	  58285	3603316	5598020	 556b44	rt-thread.elf
scons: done building targets.
```

然后，输入 `make` 命令，将 rtthread.bin 文件复制到 `tools/download_config` 目录，用于下载。

```
$ make
cp rtthread.bin ../../tools/download_config/rtthread.bin
```

## 下载

使用联发科提供的 FlashTool 工具下载。

FlashTool 下载工具位于 `SDK_ROOT/tools/flash_download_tool` 目录，请解压后使用。

FlashTool 下载的时候需要的配置文件位于 `SDK_ROOT/download_config` 目录，如下所示：

```
- bootloader.bin
- flash_download.cfg
- rtthread.bin
```

下载的时候，需要下载 `download_config` 目录下提供的默认 bootloader.bin 固件。

**注意：**

> 下载使用串口 0，下载用的波特率为 921600。  
> 下载工具配置完成后，点击 **start** 按钮开始下载，这个时候，需要先按下 RESET 按键使模块复位，然后短按 poweron 对应的按键 3 秒使模块开机。  
> 下载工具出现下载进度条，则表示正在下载，否则请再次尝试上述步骤。

## 运行

默认 RT-Thread finsh 使用串口 0，AT 使用串口 1。

程序下载完成后，使用 TTL 串口工具连接 finsh 串口和 AT 串口。然后需要先按下 RESET 按键使模块复位，然后短按 poweron 对应的按键 3 秒使模块开机。

成功运行程序后，finsh 串口会输出如下日志：

```
F1: 0000 0000
V0: 0000 0000 [0001]
00: 0006 000C
01: 0000 0000
U0: 0000 0001 [0000]
T0: 0000 00B4
Leaving the BROM

[D/FAL] (fal_flash_init:63) Flash device |           internal_flash | addr: 0x08002000 | len: 0x003ef000 | blk_size: 0x00001000 |initialized finish.
[I/FAL] ==================== FAL partition table ====================
[I/FAL] | name       | flash_dev      |   offset   |    length  |
[I/FAL] -------------------------------------------------------------
[I/FAL] | bootloader | internal_flash | 0x00000000 | 0x00010000 |
[I/FAL] | app        | internal_flash | 0x00010000 | 0x00236000 |
[I/FAL] | download   | internal_flash | 0x00246000 | 0x0015c000 |
[I/FAL] | param1     | internal_flash | 0x003a2000 | 0x00001000 |
[I/FAL] =============================================================
[I/FAL] RT-Thread Flash Abstraction Layer (V0.3.0) initialize success.
Will start app firmware.
nvdm init pass
uart reconfig pass
nvdm modem init pass
retarget freq pass
power on from <3> mode
default sleep lock pass
system initialize done@78000.

 \ | /
- RT -     Thread Operating System
 / | \     3.1.3 build Nov 22 2019
 2006 - 2019 Copyright by rt-thread team
nb_netdev init success! name:ps
[2] I/sal.skt: Socket Abstraction Layer initialize success.
[2] D/nb.onenet: nb.onenet init success
[3] I/main: Current sdk version: v1.2
[6] I/netdev: Set netdev linkup.
[6] W/netdev: Not set internet up! ip addr is any, or netdev is not up.
[7] D/main: netdev link up cb
lwIP-1.5.0 initialized!
mtk components init ok

*MATREADY: 1

+CFUN: 1
msh />
+CPIN: READY
[724] I/netdev: Set netdev linkup.
[725] D/main: netdev link up cb

+IP: 100.98.142.193
[939] I/sal.skt: Set network interface device(ps) internet status up.
[941] D/main: netdev internet up cb
```

注意，新版本 SDK 默认将 AT 串口与 Finsh 共用。如果用户希望使用 AT 指令，可以使用 finsh 命令 `at_cmd_exec`，示例如下：

```
msh />at_cmd_exec AT
AT
OK
msh />at
at_cmd_exec
msh />at_cmd_exec ATI
ATI
RealThread
NB-IOT Mobile Station
v1.2
MT2625_V01

OK
msh />
```

## 运行示例程序

在 RT-Thread finsh 串口中输入 demo_xxx 开头的命令以运行各个示例程序。

### UDP 例程使用说明

运行 demo_udp 示例如下所示：

```
msh />demo_udp
[2182] I/demo.udp: Host:118.24.21.172; Port:9000
[2183] I/demo.udp: UDP demo start
[2184] D/demo.udp: will gethostbyname...
[2184] I/demo.udp: gethostbyname pass. ip addr: 118.24.21.172
[2186] I/demo.udp: connect <118.24.21.172> success
[2189] I/demo.udp: send success
msh />[2225] I/demo.udp: received data:

This message is from NB-IOT UDP Client with RT-Thread.

[2227] I/demo.udp: UDP demo end
```

### OneNET 例程使用说明

在测试 OneNET 功能前，请确保已经在 OneNET 平台完成了 NB-IOT 产品和设备注册。参考 [OneNET 官方文档](https://open.iot.10086.cn/doc/nb-iot/book/get-start/product&device/createProduct.html)。

然后，输入命令 `demo_onenet` 启动 OneNET 连接测试。该示例程序会在 OneNet 平台创建一系列的对象，通过平台可对设备进行读、写、执行操作。详细的日志如下所示：

```
msh />demo_onenet
msh />[25912] D/nb.onenet: create object success id:3303
[25912] D/nb.onenet: create object success id:3304
[25913] D/nb.onenet: create object success id:3203
[25913] D/nb.onenet: create object success id:3311
[25913] D/nb.onenet: create object success id:3306
[25914] D/nb.onenet: create object success id:3315
[26212] D/nb.onenet: Connect success
[26462] D/nb.onenet: Reg onenet success
[26563] D/nb.onenet: find observer object(3203)
[26563] D/nb.onenet: find observer object(3203)
[26564] D/nb.onenet: find observer obj pass
[26565] D/nb.onenet: qs_cis_observer_rsp pass
[26565] D/nb.onenet: onenet_observer_rsp pass
[26767] D/nb.onenet: find observer object(3303)
[26768] D/nb.onenet: find observer obj pass
[26768] D/nb.onenet: qs_cis_observer_rsp pass
[26769] D/nb.onenet: onenet_observer_rsp pass
[26870] D/nb.onenet: find observer object(3304)
[26870] D/nb.onenet: find observer obj pass
[26871] D/nb.onenet: qs_cis_observer_rsp pass
[26872] D/nb.onenet: onenet_observer_rsp pass
[27023] D/nb.onenet: find observer object(3306)
[27023] D/nb.onenet: find observer obj pass
[27024] D/nb.onenet: qs_cis_observer_rsp pass
[27025] D/nb.onenet: onenet_observer_rsp pass
[27126] D/nb.onenet: find observer object(3311)
[27126] D/nb.onenet: find observer obj pass
[27127] D/nb.onenet: qs_cis_observer_rsp pass
[27128] D/nb.onenet: onenet_observer_rsp pass
[27279] D/nb.onenet: find observer object(3315)
[27279] D/nb.onenet: find observer obj pass
[27280] D/nb.onenet: qs_cis_observer_rsp pass
[27281] D/nb.onenet: onenet_observer_rsp pass
[28031] D/nb.onenet: onenet lwm2m connect success
[28032] D/demo.onenet: ONENET REG SUCCESS
[28033] D/nb.onenet: notify data(3303 0 5700),
[28033] D/nb.onenet: float: 75.8000 0
[28034] D/nb.onenet: notify data(3304 0 5700),
[28035] D/nb.onenet: float: 35.6000 0
[28035] D/nb.onenet: notify data(3311 0 5850),
[28036] D/nb.onenet: 0 0
[28036] D/nb.onenet: notify data(3306 0 5851),
[28037] D/nb.onenet: 12 0
[28038] D/nb.onenet: notify data(3203 0 5650),
[28038] D/nb.onenet: float: 0.5500 0
[28039] D/demo.onenet: string_len=12; opaque_len=8
[28040] D/nb.onenet: notify data(3203 0 5750),
[28040] D/nb.onenet: 303132333435 0
[28041] D/nb.onenet: notify data(3315 0 5605),
[28042] D/nb.onenet: 12345678 0
[28287] D/nb.onenet: Update success
[28288] D/nb.onenet: onenet update time success
[28689] D/nb.onenet: notify data(3303 0 5700),
[28689] D/nb.onenet: 75.8000 0
[28787] D/nb.onenet: Notify success
[28788] D/nb.onenet: ack notify to onenet success, objectid:3303        instanceid:0    resourceid:5700         msgid:189916
[28789] D/demo.onenet: notify success
```

### OneNET OTA 使用说明

在测试 OneNET OTA 功能前，请确保已经在 OneNET 平台完成了产品和设备注册。参考 [OneNET 官方文档](https://open.iot.10086.cn/doc/nb-iot/book/get-start/product&device/createProduct.html)。

如果要测试 OTA 功能，还需要添加 OTA 固件，点击查看[参考文档](https://open.iot.10086.cn/doc/nb-iot/book/ota/fota/fota_m531x_process.html#22)。

OTA 测试过程相对比较复杂，这里简单提取一下几个步骤，开发者在操作的过程中可以辅以参考：

0. 使用 env 工具，进入 **nbiot_demo** 工程，使用 scons 进行编译

    编译成功后会在 `tools/download_config` 目录下生成 `rsfota-` 开头，后加版本号的 zip 文件，注意版本号，后面 OneNET 平台会用到。这里假设版本号为 v1.1 。

    烧录 `tools/download_config` 目录下的 rtthread.bin 和 bootloader.bin 到设备中。

1. 在 OneNET 平台创建 NB-IOT 产品和设备
2. 在 OTA 升级页面，添加 OTA 群组，厂商名称写 `中国移动`，模组型号选择 **M5311**
3. 群组创建完成后，点击关联设备，选择对应 IMEI 的设备，完成关联
4. 点击 OTA 升级，进入 **固件管理** 页面
5. 点击 *添加固件版本*，目标版本选择 **步骤 0** 中的 zip 文件的版本号（假设为 v1.1），紧接着上传步骤 0 中的 zip 文件（rsfota-v1.1.zip）

    到这一步，OneNET OTA 的基本步骤已经完成，目前 OneNET 上的固件与设备中的固件版本一致。

6. 使用 env 工具的 menuconfig 功能，修改版本号，再次编译出一个 v1.2 版本的固件 `rsfota-v1.2.zip`

    ```
    - menuconfig
        - sdk config
            - (v1.2) rt-thread nbiot sdk version
    ```

7. 重复步骤 5，添加目标版本为 v1.2，固件为 rsfota-v1.2.zip 的固件版本
8. 在 步骤 7 中创建号的版本上，点击 **添加升级包**

    类型选择 **差分包**，选择当前版本为 `v1.1`，目标版本默认置为 `v1.2`，然后点击 **在线差分**，最后生成差分包。

    生成差分包后，点击测试即可。

9. 运行 v1.1 版本固件的设备
10. RT-Thread Finsh 串口输入 `demo_ota` 命令

    等待一段时间后，系统会自动检查 ota 升级。

`demo_ota` 命令日志如下：

```
msh />demo_ota
RSDL:SDK:191121_151132 RS_INFO: rs_dl_main enter
RSDL:PLATFORM:191121_151132 RS_DEBUG: start rs_create_thread 
RSDL:PLATFORM:191121_151132 RS_DEBUG: start Uadl_Task_Entry 
RSDL:PLATFORM:191121_151132 RS_DEBUG: start rs_async_get_devid 
RSDL:PLATFORM:191121_151132 RS_DEBUG: end rs_async_get_devid 
RSDL:PLATFORM:191121_151132 RS_DEBUG: xTaskCreate success  
RSDL:PLATFORM:191121_151132 RS_DEBUG: end rs_create_thread 
RSDL:SDK:191121_151132 RS_INFO: rs_sdk_init, current state = 0
RSDL:PLATFORM:191121_151132 RS_INFO: ota started -----^-^----
[-> rsfota] rsfota started ! version is v1.1
```

## 移植部分说明

- 其它模组适配

    board.h 中仅匹配了中移物联网 M5311 模块的管脚对应关系，如果用户需要适配其它模块，请修改该文件。此外，还需要替换对应的 ept 工具生成的文件。

- 模组厂商信息修改

    修改 `ports/nvdm_cfg_port.c` 文件中的 atIdentificationInfoDefaultInit 的实现。

## 注意

默认 SDK 使能了长按 powerkey 按键关机的功能，实际表现为模块重启。如果需要禁用该功能，请使用 menuconfig 配置。

```
Hardware Drivers Config  --->
    Onboard Peripheral Drivers  --->
        [*] Enable powerkey longpress shutdown mode
```
