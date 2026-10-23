# Supplied UI assets

Source: the user-provided `/Users/raj/Downloads/UI-files.zip` (23 October 2026).
`CodexUi.c` and all five `cx_font_*.c` files came from this archive.

Integration adjustments:
- Normalize the font include to `lvgl.h` for the Arduino library search path.
- In portrait, put freshness in the bottom strip to avoid overlap with the
  plan badge; landscape retains the supplied layout.
- Keep the existing `CodexUiModel`, display driver, live account/quota data,
  automatic rotation, page-transition overlap guard, Wi-Fi and Slot A controls.

Font conversion metadata in the supplied C files identifies Agency Regular
14 px labels and Agency Bold 20/29/52/80 px values. Original comments remain.
The earlier Rajdhani OFL notice does not apply to these supplied Agency assets.
The preceding Agency 42/20/82 px design is retained in the earlier checkpoint.
