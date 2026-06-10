## Context

The framebuffer driver (`Core/Src/ssd1309.c`) provides
`ssd1309_set_pixel()`/`ssd1309_get_pixel()` with bounds checking and
dirty-flag tracking, plus font-based text rendering built on top of
`ssd1309_set_pixel()`. There is no primitive yet for drawing lines, which
will be needed for separators, simple graphs/gauges, and UI chrome on
future sandbox screens.

## Goals / Non-Goals

**Goals:**
- Add `ssd1309_draw_line(p, x0, y0, x1, y1, color)` for 1px-wide lines
  between two arbitrary points, including horizontal, vertical, and
  diagonal lines in any direction.
- Reuse `ssd1309_set_pixel()` so clipping and dirty-flag tracking come for
  free, consistent with `ssd1309_draw_char`/`ssd1309_draw_string`.
- Handle degenerate cases (single point, zero-length line) without special
  casing beyond what the algorithm naturally provides.

**Non-Goals:**
- No anti-aliasing, line thickness, or dashed/dotted styles.
- No other shape primitives (rectangles, circles) — out of scope for this
  change.

## Decisions

- **Algorithm**: integer Bresenham's line algorithm, the standard choice for
  1bpp displays — no floating point, naturally handles all octants/slopes,
  and is the same approach used by common embedded graphics libraries
  (e.g. Adafruit GFX, u8g2).
- **Implementation basis**: iterate pixel-by-pixel and call
  `ssd1309_set_pixel()` for each point, rather than writing directly into
  `p->buffer`. This keeps bounds-checking and dirty-flag semantics in one
  place and matches the existing `draw_char`/`draw_string` pattern.

## Risks / Trade-offs

- [Per-pixel `ssd1309_set_pixel()` calls add small function-call overhead
  versus a hand-inlined buffer-write loop] → acceptable; lines are short
  (max ~128px) and this is not a hot path.
