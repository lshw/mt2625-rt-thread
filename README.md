# MT2625 openCPU SDK for RT-Thread 使用说明

> 请在使用该 SDK 前仔细阅读该使用说明。如果你已经了解该文档所有内容，请进一步阅读[用户指南](docs/UserGuide.md)。

本软件包是联发科 MT2625 SDK 在 RT-Thread 操作系统上的移植实现。旨在让用户基于 RT-Thread 物联网生态中丰富组件和软件包快速开发 NB-IOT 应用，降低 NB-IOT 的开发难度。

该 openCPU 版本 SDK 允许用户自定义应用程序（二次开发），用户程序运行在 MT2625 SOC 芯片上，不需要 AT 指令，不需要外接 MCU。

基于该 SDK，开发者无需关心 NB-IOT 的技术细节，使用 BSD socket 标准接口进行网络应用开发，如同开发以太网类网络应用。

请继续阅读文档来了解如何使用该 SDK。

## 工具链

该 SDK 仅支持 GCC 编译，必须使用 `gcc-arm-none-eabi-4_8-2014q3` 版本的工具链。

默认工具链位置为 SDK 根目录下的 `tools/toolchain` 文件夹中，请在使用前确认工具链是否存在。

目录结构如下：

```
SDK_ROOT
    +- tools
        +- toolchain
            +- gcc-arm-none-eabi-4_8-2014q3
```

## 编译

> 该 openCPU SDK 使用 scons 工具构建源码，需要使用 rt-thread 提供的 ENV 工具。请先正确安装 rt-thread [ENV 工具](https://www.rt-thread.org/page/download.html)。

该 SDK 为用户提供了一个基本工程，目录结构如下：

```
SDK_ROOT
    +- project
        +- nbiot_demo
```

使用 rt-thread 提供的 **ENV 命令行工具**，进入上述 `nbiot_demo` 项目目录下，运行 `scons` 命令编译，如下所示：

单核编译：

```
$ scons
```

多核编译（-j 后面的数字是电脑 CPU 的核数）：

```
scons -j8
```

## 串口配置

> **注意**，串口绑定关系不能修改。

默认串口波特率为 115200。默认的串口绑定关系如下所示：

| MT2625 串口号 | 串口功能 | 对应的 M5311 串口 |
| :---- | :---- | :---- |
| UART0        | RT-Thread Finsh | UART2 |
| UART1        | AT     | UART1 |
| UART2        | 无 | 无 |
| USB COM1     | GKI | 无 |
| USB COM2     | ULS | 无 |

## 注意

目前，该 openCPU SDK 仅适配了中移物联网 M5311 模块对 MT2625 的 GPIO 映射。如果你使用其它厂商的 MT2625 模块，需要重新映射 GPIO。如有需求，请联系 [骑士智能科技](http://www.longmain.cn)。
