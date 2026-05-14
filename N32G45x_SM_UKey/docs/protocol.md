# USB CDC 串口命令协议

## 基本格式

每条命令使用 ASCII 文本，一行一条，以 `\r\n` 或 `\n` 结束。

```text
CMD ARG1 ARG2
```

命令字大小写不敏感，例如 `info`、`Info`、`INFO` 等价；参数保持原始大小写。

返回格式：

```text
OK ...
ERR ...
WAIT ...
```

`SIGN` 和 `SM3` 的二进制数据用十六进制字符串表示。`STORE` 用于保存明文密码/通行密钥，`STORE_HEX` 用于保存十六进制编码的二进制数据。

## 命令表

| 命令 | 参数 | 说明 |
|---|---|---|
| `HELP` | 无 | 输出命令列表 |
| `INFO` | 无 | 查询设备状态 |
| `INIT` | `PIN` | 初始化设备，需 KEY1 确认 |
| `VERIFY_PIN` | `PIN` | 验证 PIN |
| `GET_PUBKEY` | 无 | 导出 SM2 公钥 |
| `SIGN` | `HEX_CHALLENGE` | 对挑战签名，需 KEY1 确认 |
| `SM3` | `HEX_DATA` | 计算 SM3 摘要 |
| `STORE` | `NAME PLAINTEXT_SECRET` | 用 SM4 加密保存一条明文密码/通行密钥，需 KEY1 确认 |
| `STORE_HEX` | `NAME HEX_DATA` | 用 SM4 加密保存一条二进制敏感数据，需 KEY1 确认 |
| `READ` | `NAME` | 解密读取敏感数据，需 KEY1 确认后明文输出 |
| `ERASE` | 无 | 擦除安全区，需 KEY1 确认 |
| `KEY1` | 无 | 仿真确认键 |
| `KEY2` | 无 | 仿真取消键 |
| `KEY3` | 无 | 仿真模式键 |

## 状态机

```text
UNINIT
  INIT + KEY1 -> LOCKED

LOCKED
  VERIFY_PIN 成功 -> AUTHED
  VERIFY_PIN 失败 3 次 -> PIN_LOCKED

AUTHED
  SIGN + KEY1 -> AUTHED
  STORE/STORE_HEX/READ + KEY1 -> AUTHED
  SM3 -> AUTHED

PIN_LOCKED
  只能 ERASE + KEY1 恢复出厂
```

## 按键分配

| 按键 | 作用 |
|---|---|
| KEY1 | 用户确认/授权 |
| KEY2 | 取消当前等待操作 |
| KEY3 | 模式切换或状态显示 |
