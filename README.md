# Codex Dashboard — ESP32-S3

Standalone Codex usage monitor for the Waveshare ESP32-S3-LCD-1.47B family: a 172×320 ST7789 LCD, 16 MiB flash and 8 MiB OPI PSRAM. After Wi-Fi and account setup, it runs independently of a computer.

## Current design

The LCD and responsive webpage use Agency typography, a large blue **5h** panel, compact purple **Weekly** panel, segmented meters, reset countdowns and an orange chip-temperature tile. Percentages show allowance **left**. Portrait and phone layouts retain the five-hour focus.

The LCD uses LVGL 8.3.11 and motion-based rotation. Tap BOOT for device details; hold it for three seconds and release to select ESPINFO in Slot A. Display updates run separately from HTTPS requests. The firmware uses an 80 MHz CPU, reduced backlight brightness and Wi-Fi modem sleep; these settings do not establish a thermal limit. Chip temperature is not ambient temperature.

- LCD layout: `src/CodexUi.c`, model: `src/CodexUi.h`.
- Web layout: `web/dashboard.html` and `web/style.css`.
- Fonts: `src/cx_font_*.c` and `web/fonts/`; provenance is documented alongside them.
- Generated web assets: `src/WebAssets.h` and `src/WebFonts.h`, produced by `scripts/pack_web.py`.
- The UI is hand-authored C, not a SquareLine Studio project.

## Setup

1. Use the setup Wi-Fi name and password shown on your own board's LCD.
2. Open the setup address shown on the LCD and save a 2.4 GHz Wi-Fi network.
3. After connection, tap BOOT to see the assigned address and open it in a browser.
4. In **Account credentials**, supply your own locally obtained Codex OAuth access token and account ID directly on the device page. Never commit or share them.
5. Saved account credentials load automatically after restart. Older passphrase records request a one-time migration.
6. In **Device settings → Wi-Fi networks**, add up to five networks and select their preferred order. Blank passwords retain an existing password; explicit open-network selection removes it.

Saved networks are tried in priority order. Association allows 25 seconds; an associated router gets up to 90 seconds for DHCP. If all networks fail, setup mode opens and retries continue. Changing preferred order takes effect on the next connection; selecting Connect reconnects immediately.

## Application slots

This project targets the verified two-application layout in `partitions.csv`:

| Slot | Address | Capacity | Application |
|---|---|---|---|
| A / app0 | 0x10000 | 3 MiB | Existing ESPINFO |
| B / app1 | 0x310000 | 3 MiB | Codex Dashboard |

Check your actual board layout and make private recovery backups before installation. Flash only the application image at **0x310000**, maximum **0x300000 bytes**, when this layout is confirmed. Do not flash generated merged, bootloader or partition images over an existing dual-app installation. Ordinary PlatformIO upload is blocked by this project.

The physical BOOT gesture, webpage button and USB `BOOT_A` command select Slot A. To select an existing application from the ROM loader:

```sh
python3 scripts/slot_tool.py B --port /dev/cu.YOUR_LOADER_PORT --enter-loader
```

The helper supports macOS, checks the partition table and image header, backs up selection metadata and updates only the inactive selection sector. It never writes an application or NVS. Use `A` to select Slot A. If automatic loader entry fails, hold BOOT, press/release RESET, release BOOT, then use the detected loader port without `--enter-loader`. Selection persists across restart.

## Build

Pinned dependencies: Arduino ESP32 3.3.12, LovyanGFX 1.2.32, ArduinoJson 7.4.2 and LVGL 8.3.11. The local build helper expects a compatible toolchain; specify its directory with `--toolchain PATH`. Libraries belong under the Git-ignored `libraries/` directory.

```sh
mkdir -p libraries
curl --fail --location --output /tmp/codex-arduinojson.zip \
  https://codeload.github.com/bblanchon/ArduinoJson/zip/refs/tags/v7.4.2
unzip -q /tmp/codex-arduinojson.zip -d libraries
curl --fail --location --output /tmp/codex-lvgl.tar.gz \
  https://codeload.github.com/lvgl/lvgl/tar.gz/refs/tags/v8.3.11
tar -xzf /tmp/codex-lvgl.tar.gz -C libraries
python3 scripts/build.py --toolchain /path/to/BuildTools
python3 scripts/test_slots.py
```

Output: `build/arduino/CodexDashboard.ino.bin`. Only this app image belongs in Slot B. PlatformIO is an alternative configuration that remains unverified. The GitHub workflow builds the pinned Arduino target and packages an app-only artifact.

## Preview and validation

```sh
python3 scripts/preview_web.py
python3 scripts/render_lvgl_design.py --fetch
c++ -std=c++17 tests/orientation.cpp -o /tmp/codex-orientation-tests
/tmp/codex-orientation-tests
```

The web preview runs at http://127.0.0.1:8765 with synthetic data. Query cases `full`, `empty`, `missing`, `setup`, `error` and `long` exercise visual edge cases. Preview network values are fictional documentation examples. The host LVGL renderer checks production layout geometry, orientation changes and overlapping transitions.

USB diagnostics include `STATUS`, `SCREEN`, `DETAILS`, `PREVIEW 0..3`, `LIVE`, `TLS_TEST`, `STORAGE_TEST`, `WIFI_DIAG`, `WIFI_CONNECT <SSID>`, `WIFI_RESTORE` and `RESTART`. Preview mode never changes account or allowance data. Screenshots can include setup credentials when displayed; keep diagnostic captures private.

## Account and network security

The firmware polls the undocumented Codex usage endpoint approximately every three minutes. Its schema and token acceptance may change. OAuth refresh and an embedded sign-in flow are not implemented. The firmware never requests your ChatGPT password.

Account data is encrypted with AES-256-GCM using a random device key stored in NVS. This permits automatic startup but does not protect against an attacker who can read flash. Legacy passphrase migration verifies the replacement record before deleting the old one. Wi-Fi profiles use a separate namespace. Management status excludes tokens and Wi-Fi passwords.

The device webpage uses HTTP on a trusted local network; do not expose it to the internet. Keep credentials, NVS images, recovery backups and deployment logs outside Git. The ignored `.local/`, `backups/` and `build/` directories retain private working artifacts.

See `PROGRESS.md` for the public development checkpoint and font-source documents for asset provenance. Agency assets are not covered by the historical Rajdhani OFL notice.
