# Production LVGL preview and layout checks

The user approved the Instrument panel preview. The host renderer now uses the
same `src/CodexUi.c` widgets and generated fonts as the Slot B firmware.

The screens are rendered by actual LVGL 8.3.11 using native labels, panels,
segmented meters and a 192 ms screen-load slide transition. The preview uses
sample values and sample device details. It is hand-authored C, not a
SquareLine Studio export or an editable `.spj` project.

`ui_preview.c` supplies sample models to the production widget builder.
`render_preview.c` runs a headless LVGL display on the host and captures RGB565
frames. `lv_conf.h` is only for the host preview. The firmware uses `src/lv_conf.h` with its heap in PSRAM; the host uses a static heap.

## Reproduce

```sh
python3 scripts/render_lvgl_design.py --fetch
```

A C compiler is required. The pinned official LVGL source is downloaded to
`.local/lvgl-design` if absent. Subsequent builds need no network. The program
checks object containment for all six layouts (0, mixed and 100% values, in
landscape and portrait). Images and page-transition frames are saved under
`build/lvgl-preview/`.

## Typography and reference

The current production layout and all five Agency fonts were supplied by the
user in `UI-files.zip`. Font roles are 14 px labels, 20 px values, 29 px percent
sign, 52 px Weekly number and 80 px primary 5h number. See
`src/UI-ASSET-SOURCE.md` and `design/supplied-ui/source-manifest.json`.

The import keeps the supplied landscape design. In portrait, freshness moves
from the header into the footer to avoid the plan badge. Native board drivers
and the transition-overlap guard remain unchanged. Earlier Agency instrument
proposals are retained under `design/agency/` and in private checkpoints.

## Review status

Design approved on 23 October. See `PROGRESS.md` for installation and hardware
validation. Slot A and saved account/Wi-Fi data are preserved by app-only updates.
