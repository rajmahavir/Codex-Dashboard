# Codex Dashboard — Standalone ESP32-S3 Usage Monitor

Firmware for a Waveshare ESP32-S3-LCD-1.47B-style board with a 172×320 ST7789 display. After flashing, it joins your 2.4 GHz Wi-Fi network, hosts a local web dashboard, and displays the usage windows on the LCD. No always-on Mac or Ubuntu server is needed.

## Build status

A successful GitHub Actions build completed on 2026-10-22. The build produced `firmware.bin`, `bootloader.bin`, `partitions.bin`, and `firmware.elf`.

Successful build: https://github.com/rajmahavir/Codex-Dashboard/actions/runs/39747972792

The artifact is named `codex-dashboard-esp32s3-firmware`. Open the workflow run above and download it under **Artifacts**. Artifacts expire after 30 days, but future successful runs publish a fresh one.

**Important:** `firmware.bin` is the application image, not a full factory image. Do not flash that single file at offset zero. Prefer PlatformIO's Upload target, which uses the matching board configuration and flash offsets. This is compile-verified; it has not yet been physically flashed or tested on the target board.

## Build with PlatformIO

1. Install VS Code and the PlatformIO extension.
2. Clone/download the project.
3. Fetch the pinned CA certificate bundle header, then build:

```sh
curl --fail --location --retry 3 --output src/esp32_cert_bundle.h \
  https://raw.githubusercontent.com/tanakamasayuki/ESP32CertBundle/30d77cfc6daae72e6c8afc332eef0c0f1409c637/src/esp32_cert_bundle.h
pio run -e esp32-s3-devkitc-1
```

To flash over USB, use `pio run -e esp32-s3-devkitc-1 -t upload`.

## First boot setup

1. Open the serial monitor at 115200 baud.
2. On first boot, the device starts a Wi-Fi access point named like `CodexSetup-A1B2C3`, with password `Setup-A1B2C3`. The actual values are printed on serial and the LCD.
3. Connect your phone/computer to the AP and visit `http://192.168.4.1`.
4. Enter your 2.4 GHz Wi-Fi credentials. After restart, open the device's LAN IP address (shown on serial and the LCD).
5. Open **Configure Codex credentials** and enter the access token and account ID obtained from your own local Codex authentication session. Set an encryption passphrase of at least 10 characters.

## Authentication caveat

There is no verified supported embedded “Sign in with ChatGPT” flow in this firmware. It does not request or store your ChatGPT password. It expects a Codex OAuth access token and account ID obtained locally from an existing Codex login. Authentication may be stored in `~/.codex/auth.json` or in the operating system's credential store, depending on setup. Never upload `auth.json`, paste tokens in a chat, or share them. Token refresh is not implemented.

The device polls the undocumented endpoint `https://chatgpt.com/backend-api/wham/usage` about every three minutes. The endpoint and response shape can change, reject tokens, or return only one usage window. A successful build does not verify live authentication or API operation.

## Security

- The token and account ID are encrypted in ESP32 NVS with AES-256-GCM; PBKDF2-HMAC-SHA256 derives the key from the passphrase. The passphrase itself is not stored, so it must be re-entered after reboot.
- Wi-Fi credentials are stored separately in ESP32 Preferences/NVS.
- The web UI uses HTTP on your local network. Do not port-forward it to the internet; use it only on a trusted LAN.
- This is not hardware-backed secure storage. Flash encryption and secure boot are not enabled, and physical access to the board remains a risk.

## Hardware

ST7789, 172×320. Display pins used: MOSI 45, SCLK 40, CS 42, DC 41, RST 39, backlight 46. Libraries: TFT_eSPI and ArduinoJson.
