# N32G45x Keil 移植说明

## 工程建议

优先从厂商官方 `USB CDC Virtual COM` 示例工程开始，而不是从空工程手写 USB 描述符。这样 Windows 侧可直接识别为虚拟串口，便于用 SSCOM 调试。

当前已经生成一个可打开的 Keil 工程副本：

```text
Nationstech.N32G45x_Library.2.2.0/projects/n32g45x_EVAL/examples/USB/SM_UKey/MDK-ARM/virtual_com_port.uvprojx
```

需要加入的应用层文件：

```text
firmware/app/app_config.h
firmware/app/app_types.h
firmware/app/crypto_port.h
firmware/app/platform_port.h
firmware/app/secure_store.c
firmware/app/secure_store.h
firmware/app/ukey_protocol.c
firmware/app/ukey_protocol.h
firmware/app/ukey_state.c
firmware/app/ukey_state.h
firmware/app/utils_hex.c
firmware/app/utils_hex.h
```

## 必须实现的底层函数

`crypto_port.h`：

- `crypto_sm3()`
- `crypto_sm4_encrypt_cbc()`
- `crypto_sm4_decrypt_cbc()`
- `crypto_sm2_generate_keypair()`
- `crypto_sm2_sign()`

`platform_port.h`：

- `platform_flash_load_secure_area()`
- `platform_flash_save_secure_area()`
- `platform_random_bytes()`
- `platform_device_id()`

## USB CDC 接入方式

CDC 接收回调中缓存字节，遇到换行后调用：

```c
ukey_on_command(line, out, sizeof(out));
cdc_send_string(out);
cdc_send_string("\r\n");
```

按键中断或轮询中调用：

```c
ukey_on_key(UKEY_KEY_CONFIRM, out, sizeof(out)); // KEY1
ukey_on_key(UKEY_KEY_CANCEL, out, sizeof(out));  // KEY2
ukey_on_key(UKEY_KEY_MODE, out, sizeof(out));    // KEY3
```

## 关键答辩点

本设计没有把私钥通过 USB 导出。上位机只能提交挑战值，开发板在用户按 KEY1 授权后使用 SM2 私钥签名。这体现了 U 盾“私钥不出设备”的安全边界。
