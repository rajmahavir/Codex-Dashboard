# Development checkpoint — v3.3

The LCD and responsive webpage share the supplied instrument design: Agency fonts, a large blue 5h panel, purple Weekly panel, segmented allowance meters and reset tiles. Both show percentage left. Portrait layouts retain the five-hour focus.

Implemented: LVGL 8.3.11 display task, motion-based rotation, BOOT page switching, automatic credential loading, up to five saved Wi-Fi networks and locally served webfonts. Slow network requests do not block display updates.

Validation completed: Arduino application build, five slot-tool tests, host layout and transition checks, real-board rotation and application read-back, and desktop/mobile browser checks including missing and extreme allowance values. The existing Slot A application was preserved during installation. Remote CI and PlatformIO remain unverified at this checkpoint.

Editable sources: `src/CodexUi.c`, `web/dashboard.html`, and `web/style.css`. See README.md for build and preview instructions. Private deployment history and recovery artifacts are retained locally and excluded from this public repository.
