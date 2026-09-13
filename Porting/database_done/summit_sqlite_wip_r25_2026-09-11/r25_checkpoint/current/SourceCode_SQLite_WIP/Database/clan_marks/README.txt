Summit local clan-mark storage
==============================

The server-side legacy clan bridge reads clan marks from this directory instead
of downloading them from /ClanContent over HTTP.

Filename contract:
    <legacy ClanMark identifier>.bmp

Examples are intentionally not invented here: the exact identifier is the value
already carried by CL.MIconCnt / clan protocol formatting and previously used as
the basename in the legacy HTTP URL.

A missing file is reported to the legacy caller as mark-read result 2, matching
the old non-BMP/not-available path. The default mark required by clan creation is
still unresolved and is not synthesized by this checkpoint.
