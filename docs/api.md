
## 模组信息类

### 获取厂商信息

> char *rt_nb_manufacturer_id_get(void);

### 获取模组 ID

> char *rt_nb_model_id_get(void);

### 获取 SDK 软件版本

该 SDK 软件版本用于 rsfota 升级，进行版本比较。

> char *rt_nb_sw_ver_get(void);

### 获取硬件版本

> char *rt_nb_hw_ver_get(void);

### 查询 IMEI

> char *rt_nb_imei_get(void);

### 查询 IMSI

> char *rt_nb_imsi_get(void);

### 查询 ICCID

> char *rt_nb_iccid_get(void);

### 查询 SN

> char *rt_nb_sn_get(void);

## 网络服务类

### 查询 CSQ 信号质量

> rt_err_t rt_nb_csq_get(int32_t *rssi, int32_t *ber);

### 设置 CFUN

```
rt_err_t rt_nb_cfun_set_get(int32_t *status, uint8_t direction);
```

`rt_nb_cfun_set_get` 接口用于设置或获取 CFUN 模式。

当 `direction` 参数为 0 时，读取当前 CFUN 模式。

当 `direction` 参数为 1 时，设置 CFUN 为 `status` 指针参数指向的值。

当设置 CFUN 模式时，可选的配置如下所示：

- 0：最小功能
- 1：全功能（默认）
- 4：只禁用模组的射频接收发射功能
- 7：禁用 SIM 卡；发送接收电路有效

### 查询 SIM 卡状态

> rt_err_t rt_nb_sim_card_status_get(uint8_t *status);

参数 status 是入参，用于接收 SIM 卡状态。`status = 1` 标识 SIM 卡就绪，`status = 0` 标识 SIM 卡未就绪。

### 查询网络状态

> rt_err_t rt_nb_cereg_get(rt_nb_network_registration_status_rsp_t  *cereg);

> 对应 AT 指令 `+CEREG`

```
typedef struct {
    int32_t n;              /* 如果 n 为 0，表示禁用提示 */
    int32_t stat;           /* EPS 网络注册状态，1 表示已注册，0 表示未注册 */
    int32_t tac;            /* 跟踪区号，两字节数字的十六进制表示 */
    int32_t ci;             /* 字符串型，四字节的十六进制格式的小区 ID */
    int32_t act;            /* 表示接入的网络类型，9 表示 NB-IOT 网络 */
    int32_t rac;            /* NB-IOT 中该值为 0x00 */
    int32_t cause_type;     /* 整数类型，表示拒绝类型 */
    int32_t reject_cause;   /* 整数类型，包含注册失败的原因。 */
    int32_t active_time;    /* 指示在 e-utran 中分配给 ue 的活动时间值（t3324），比特编码，编码格式见后面的描述 */
    int32_t periodic_tau;   /* 指示在e-utran中分配给ue的扩展周期tau值（t3412），比特编码，编码格式见后面的描述 */
} rt_nb_network_registration_status_rsp_t;
```

**active_time 值的编码格式：**

使用一个字节的比特编码来表示对应的值。

一个字节共 8 位（bit0-bit7），其中高 3 位 bit7-bit6-bit5 表示单位，低 5 位表示对应的值。

单位的取值范围如下：

- `000`：2 秒
- `001`：1 分钟
- `010`：6 分钟
- `111`：T3324 计时器无效

示例：`00100111`

其中，高三位 `001` 代表单位为 1 分钟；
另外，低五位 `00111` 对应十进制为 `7`，也就是有 7 个单位。因此，该 `00100111` 编码表示的值为 `7 * 1 分钟 = 7 分钟`。

**periodic_tau 值的编码格式：**

periodic_tau 的编码格式跟 active_time 一致，区别是高三位表示的单位不一样。

单位的取值范围如下：

- `000`：10 分钟
- `001`：1 小时
- `010`：10 小时
- `011`：2 秒
- `100`：30 秒
- `101`：1 分钟
- `110`：320 小时
- `111`：T3412 计时器无效

示例：`01000111`

其中，高三位 `010` 代表单位 10 小时；
另外，低五位 `00111` 对应十进制为 `7`，也就是有 7 个单位。因此，该 `01000111` 编码表示的值为 `7 * 10 小时 = 70 小时`。

**EPS 网络状态 stat：**

- 0：没有注册到网络，当前没有进行网络搜索
- 1：已经注册到网络
- 2：没有注册到网络，当前正在搜索网络
- 3：注册被拒绝
- 4：未知状态
- 5：已经注册到网络，漫游状态
- 6：注册到“仅限短信”，仅当 act 为 9（NB-IOT）的时候适用
- 7：注册到“仅限短信”，处于漫游状态，仅当 act 为 9（NB-IOT）的时候适用

### 设置 CEREG 返回类型

> rt_err_t rt_nb_cereg_set(uint8_t n);

通过该接口设置 `rt_nb_cereg_get` 查询时返回的内容。

参数 n 可以为以下值，不同的值对应不同的返回内容：

- 0 或 1: `<n>,<stat>`
- 2: `<n>,<stat>[,[<tac>],[<ci>],[<act>],[<rac>]]`
- 3: `<n>,<stat>[,[<tac>],[<ci>],[<act>],[<rac>],[<cause_type>,<reject_cause>]]`
- 4: `<n>,<stat>[,[<tac>],[<ci>],[<act>],[<rac>][,,[,[<Active-Time>],[<Periodic-RAU>],[<GPRS-READY-timer>]]]]`
- 5: `<n>,<stat>[,[<tac>],[<ci>],[<act>],[<rac>][,[<cause_type>],[<reject_cause>][,[<Active-Time>],[<Periodic-TAU>]]]]]`

### 设置 DNS

适用 menuconfig 配置 DNS。

## 低功耗类

### 设置低功耗进入退出回调

回调函数定义：

```
typedef void (*hal_sleep_manager_callback_t)(void *data, uint32_t mode);
```

回调函数参数说明：

- data：回调函数用户数据，指针类型
- mode：低功耗模式

可选的低功耗模式（mode 可选的取值）：

- 0：none sleep 模式
- 1：idle sleep 模式
- 2：light sleep 模式（轻度睡眠模式）
- 4：deep sleep 模式（深度睡眠模式）
- 8：deeper sleep 模式（保持模式）

> 注意，没有深度睡眠模式的退出回调。因为深度睡眠模式唤醒后相当于重启系统，因此配置深度睡眠退出回调无意义。

回调设置函数：

```
注册低功耗进入回调：
void hal_sleep_manager_register_suspend_callback(hal_sleep_manager_callback_t callback, void *data, uint32_t mode);

注册低功耗退出回调：
void hal_sleep_manager_register_resume_callback(hal_sleep_manager_callback_t callback, void *data, uint32_t mode);
```

## 硬件适配类

### 查询启动模式

> rtc_power_on_result_t rtc_power_on_result_external();

```
typedef enum {
    FIRST_BOOT_UP       = 0,     /* 冷启动（上电启动） */
    DEEP_SLEEP          = 1,     /* 从 deep sleep 模式下唤醒启动 */
    DEEPER_SLEEP        = 2,     /* 从 deeper sleep 模式下唤醒启动 */
    SYS_RESET           = 3,     /* 长按关机模式下启动，或者 sys_reset */
    WDT_HW_RESET        = 4,     /* 看门狗硬复位 */
    WDT_SW_RESET        = 5,     /* 看门狗软复位 */
    FORCED_SHUT_DOWN    = 6,     /* 强制关机 */
    FORCED_RESET        = 7,     /* 强制复位 */
    POWER_ON_RESULT_MAX = 7
} rtc_power_on_result_t;
```

### 配置 powerkey 功能

```
uint8_t pwkey_long_press_shutdown_ctrl(uint8_t ctrl);
```

`pwkey_long_press_shutdown_ctrl` 接口用于配置是否使能长按关机功能，默认使能。

当前 SDK 版本，长按 pwkey 8 秒会重启系统。

参数 `ctrl` 的取值与功能：

- 0：关闭长按 pwk 关机的功能
- 1：打开长按 pwk 关机的功能（默认启动）
- 2：读取当前的 pwk 关机功能的配置状态

**如何配置：**

使用 menuconfig 配置该功能是否使能。

```
Hardware Drivers Config  --->
    Onboard Peripheral Drivers  --->
        [*] Enable powerkey longpress shutdown mode
```
