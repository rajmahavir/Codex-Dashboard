# Codex Dashboard — ESP32-S3

This repository builds firmware for the ESP32-S3 with a 172×320 ST7789 display (Waveshare 1.47-inch style pin map).

## Build firmware

Every push to `main`/`master`, and manual workflow dispatch, starts the GitHub Actions build. Open **Actions → Build ESP32-S3 firmware**, select a successful run, and download the `codex-dashboard-esp32s3-firmware` artifact. The ZIP contains `firmware.bin` and available companion files.

## Important

The initial build configuration is a build scaffold. Do not flash it as a functional Codex usage dashboard until `src/main.cpp` has been added and the workflow completes successfully. Board pin maps, flash size, display driver, and partition layout should be verified for your exact board revision before flashing.
