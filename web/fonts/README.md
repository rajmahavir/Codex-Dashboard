# Embedded Agency webfonts

These fonts match the supplied LCD design. Regular derives from
https://github.com/VolosR/M5StickS3/blob/main/M5StickS3/assets/AGENCYR.TTF
and bold from
https://github.com/VolosR/WaveShareC6lvglexample/blob/main/NewYearExample/assets/AGENCYB.TTF .
The source TTFs are cached locally in `.local/reference-fonts/`.

The WOFF2 files were generated with fontTools 4.67.0 and Brotli 1.2.0, retaining
name/copyright metadata, Latin-1 and the needed punctuation. They are served
from the board's flash at `/fonts/agency-{regular,bold}-v1.woff2`; no external
font service is contacted. `scripts/pack_web.py` generates `src/WebFonts.h`.
Font files use versioned URLs and immutable cache headers. Change the URL
version when changing their content. The earlier Rajdhani OFL notice does not
apply to these Agency assets.
