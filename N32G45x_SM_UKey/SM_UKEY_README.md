# SM UKey Keil 工程说明

本工程由官方 `USB/Virtual_COM_Port` 示例复制改造而来，保留 USB CDC 虚拟串口框架，把原来的 `USB <-> USART1` 透传逻辑替换为 `USB <-> 国密 U 盾命令解析器`。

打开工程：

```text
MDK-ARM/virtual_com_port.uvprojx
```

## 已接入内容

- `ukey_app/`：命令协议、PIN 状态机、Flash 安全区、十六进制工具。
- `ukey_port/ukey_usb_bridge.c`：USB CDC 字节流到命令行协议的桥接层。
- `ukey_port/crypto_n32g45x.c`：密码算法适配层。
- `ukey_port/platform_n32g45x.c`：Flash、随机数、设备 ID 适配层。
- `ukey_port/software_sm4.c`：标准软件 SM4-CBC 实现。
- `ukey_port/software_sm2.c`：软件 SM2 接口门面。

## 官方库使用情况

本版本官方库 `Nationstech.N32G45x_Library.2.2.0` 中可确认提供：

- `n32g45x_hash.lib`：用于 `SM3_Hash()`。
- `n32g45x_rng.lib`：用于 `GetTrueRand_U32()`。

Keil 工程已经加入：

```text
firmware/n32g45x_algo_lib/inc
firmware/n32g45x_algo_lib/lib/n32g45x_hash.lib
firmware/n32g45x_algo_lib/lib/n32g45x_rng.lib
```

## SSCOM 测试

烧录后，电脑识别出 COM 口，SSCOM 发送命令时建议勾选“发送新行”。

```text
INFO
INIT 123456
KEY1
VERIFY_PIN 123456
GET_PUBKEY
SIGN 11223344556677889900AABBCCDDEEFF
KEY1
SM3 616263
STORE TOKEN 4142434431323334
READ TOKEN
```

当前没有接物理按键中断时，可以直接发送 `KEY1`、`KEY2`、`KEY3` 命令模拟按键。

## 重要边界

`software_sm4.c` 是标准 SM4 实现，已用公开测试向量验证。

`software_sm2.c` 当前是可替换的软件 SM2 门面：它保持“私钥不导出、授权后签名”的工程流程，但签名值由 `SM3(private || message)` 派生，不能作为标准 SM2 签名用于第三方验签。后续如果加入完整 SM2/ECC 软件库，只需要替换该文件，不需要改命令协议和 USB 状态机。

