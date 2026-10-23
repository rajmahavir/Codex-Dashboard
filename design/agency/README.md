# Five-hour focus — reference-font preview

This off-board LVGL proposal uses the actual Agency Bold fonts from the user's
Volos reference, replacing the previous Rajdhani approximation. The dominant
5h readout occupies roughly two-thirds of landscape; Weekly is secondary.
Portrait stacks a large 5h section above a compact Weekly instrument.
Values, resets, temperature and connection details are synthetic examples.
The user approved this proposal. Production integration uses `src/CodexUi.c`;
see `PROGRESS.md` for the final installation and verification state.

Font source: https://github.com/VolosR/WaveShareC6lvglexample/tree/main/NewYearExample

- `ui_font_BIG.c`: Agency Bold, 42 px, 1 bit per pixel (original asset).
- `ui_font_MID.c`: Agency Bold, 20 px, 1 bit per pixel (original asset).
- `assets/AGENCYB.TTF`: the same source typeface; converted at 82 px for the
  primary number with lv_font_conv 1.5.3, 1 bpp, digits and dash only.
- Small interface text: Montserrat, as used in the reference screen source.

Original reference fonts remain in ignored `.local/reference-fonts/`.
The embedded C font assets are now in `src/codex_agency_big.c`,
`src/codex_agency_mid.c` and `src/codex_agency_hero.c`.
Original conversion comments and provenance are preserved in the source assets
and `src/AGENCY-FONT-SOURCE.md`. The original proposal renderer below retains
its fixed example data; `scripts/render_lvgl_design.py` tests the dynamic
production layout, including setup, device details and animation lifetimes.

## Render

After running the main host preview to build pinned LVGL 8.3.11:

```
python3 scripts/render_lvgl_design.py --fetch
python3 design/agency/render.py
```

The second command expects the original reference font C assets locally and
`agency_hero.c`, generated from `AGENCYB.TTF` using:

```
node .local/lvgl-design/node_modules/lv_font_conv/lv_font_conv.js \
  --font .local/reference-fonts/AGENCYB.TTF --symbols '0123456789-' \
  --size 82 --bpp 1 --format lvgl --no-compress --lv-include lvgl.h \
  --lv-font-name agency_hero --output .local/reference-fonts/agency_hero.c
```

Previews are saved under `build/agency-preview/`. The renderer checks child
bounds at 0%, mixed and 100% values in landscape, plus mixed and 100% portrait.
