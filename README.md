# SM UKey Simulator for N32G45x

基于 N32G45x 的国密 U 盾仿真器课程设计工程。项目从 Nationstech 官方 USB CDC 虚拟串口 demo 精简而来，通过电脑 COM 口与开发板通信，模拟 U 盾的初始化、PIN 验证、挑战签名、SM3 摘要和 Flash 加密存储流程。

## Features

- USB CDC virtual COM communication for SSCOM-style testing.
- Text command protocol: `INFO`, `INIT`, `VERIFY_PIN`, `GET_PUBKEY`, `SIGN`, `SM3`, `STORE`, `READ`, `ERASE`.
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
STORE TOKEN 4142434431323334
READ TOKEN
```

`KEY1`, `KEY2`, and `KEY3` can be sent as text commands to simulate board buttons before GPIO key handling is wired in.

## Notes

`software_sm4.c` is a standard SM4-CBC implementation. `software_sm2.c` currently preserves the UKey signing workflow as a replaceable software facade; it is not a full standard SM2/ECC implementation yet.

Detailed notes are available in:

```text
N32G45x_SM_UKey/SM_UKEY_README.md
N32G45x_SM_UKey/docs/protocol.md
```
