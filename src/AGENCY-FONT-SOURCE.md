Historical v3.1 font notes. The previous source assets were archived in
`.local/agency-source-before-import/`; the current user-supplied font assets
are documented in `UI-ASSET-SOURCE.md`.

# Agency Bold font source

The user requested the exact font from this reference:
https://github.com/VolosR/WaveShareC6lvglexample/tree/main/NewYearExample

`codex_agency_big.c` and `codex_agency_mid.c` retain the original 42 px and
20 px, 1 bpp glyph assets (only the include changed to CodexUi.h).
`codex_agency_hero.c` uses the same assets/AGENCYB.TTF converted at 82 px,
1 bpp, digits and dash only, with lv_font_conv 1.5.3.
Original conversion comments are preserved. This is separate from the OFL
Rajdhani font previously used; FONT-LICENSE.txt applies to that previous font.
