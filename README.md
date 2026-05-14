# SM UKey Simulator for N32G45x

基于 N32G45x 的国密 U 盾仿真器课程设计工程。项目从 Nationstech 官方 USB CDC 虚拟串口 demo 精简而来，通过电脑 COM 口与开发板通信，模拟 U 盾的初始化、PIN 验证、挑战签名、SM3 摘要和 Flash 加密存储流程。

## Features

- USB CDC virtual COM communication for SSCOM-style testing.
- Text command protocol: `INFO`, `INIT`, `VERIFY_PIN`, `GET_PUBKEY`, `SIGN`, `SM3`, `STORE`, `STORE_HEX`, `READ`, `ERASE`.
- Physical button support: `KEY1` is PA4 confirm/authorize, `KEY2` is PA5 cancel. The text commands `KEY1` and `KEY2` are still available for simulation.
- UKey-like state machine with PIN retry limit and user-confirmed sensitive operations.
- SM3 through the N32G45x official algorithm library.
- True random generation through the N32G45x official RNG library.
- Software SM4-CBC implementation for encrypted secure storage.
- Replaceable software SM2 facade for the private-key-in-device signing workflow.
- Trimmed Keil MDK project with only the required CMSIS, peripheral, USBFS, and algorithm files.

## Project Layout

```text
N32G45x_SM_UKey/
├─ app/          Protocol parser, state machine, secure storage
├─ port/         N32G45x adapters, USB bridge, software SM2/SM4
├─ user/         USB CDC application files from the official demo
├─ libraries/    Required CMSIS, standard peripheral, USBFS, and algo libs
├─ MDK-ARM/      Keil MDK project
├─ docs/         Protocol and porting notes
└─ tools/        Optional PC serial client
```

## Build

Open this project in Keil MDK:

```text
N32G45x_SM_UKey/MDK-ARM/virtual_com_port.uvprojx
```

The trimmed project has been verified with ARMCC V5.06:

```text
0 Error(s), 0 Warning(s)
```

## SSCOM Test Flow

After flashing, open the enumerated COM port and send one command per line:

```text
INFO
INIT 123456
KEY1
VERIFY_PIN 123456
GET_PUBKEY
SIGN 11223344556677889900AABBCCDDEEFF
KEY1
SM3 616263
STORE TOKEN MyPasskey-123456
KEY1
READ TOKEN
KEY1
```

`STORE` saves plaintext password/passkey-style secrets. `STORE_HEX` saves binary data encoded as hex. Both `STORE` and `READ` require physical confirmation with `KEY1` before Flash write or plaintext output. `KEY1` and `KEY2` can be pressed on the board after a `WAIT ...` response. They are mapped to PA4 and PA5 by default, active-low with internal pull-ups. The same names can still be sent as text commands for testing.

## Notes

`software_sm4.c` is a standard SM4-CBC implementation. `software_sm2.c` currently preserves the UKey signing workflow as a replaceable software facade; it is not a full standard SM2/ECC implementation yet.

Detailed notes are available in:

```text
N32G45x_SM_UKey/SM_UKEY_README.md
N32G45x_SM_UKey/docs/protocol.md
```
