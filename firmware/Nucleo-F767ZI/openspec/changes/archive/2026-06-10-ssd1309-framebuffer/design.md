## Context

`ssd1309_bringup()` (from `ssd1309-display-bringup`) powers on the panel and
writes a hard-coded all-`0xFF` test pattern directly to GDDRAM, page by
page, using `HAL_I2C_Mem_Write` with the data control byte (`0x40`). There is
no in-RAM representation of display content — every "frame" is generated
on the fly. This change adds a real framebuffer so callers can draw pixels
and text, then push the result to the panel.

The SSD1309 GDDRAM (page-addressing mode, as already configured by
`ssd1309_bringup()`'s `0x20 0x02`) is organized as 8 pages x 128 columns,
each byte covering 8 vertical pixels (one column of one page), LSB = top
row of the page (datasheet Section 8.6/Figure 8-13). A 1bpp framebuffer in
the same layout can be flushed with a near-identical write loop to the one
already used for the test pattern.

## Goals / Non-Goals

**Goals:**
- In-RAM 1bpp framebuffer matching SSD1309 page-major GDDRAM layout.
- Pixel-level primitives: clear, fill, set/get pixel.
- `ssd1309_flush()` to push the whole buffer to the panel.
- Built-in fixed-width font + `draw_char`/`draw_string` for debug text.

**Non-Goals:**
- No per-page/region dirty tracking — only a single whole-buffer dirty
  flag (per design decision below). When dirty, `flush()` always writes
  the full buffer; it never writes a subset of pages.
- No line/circle/rect drawing primitives beyond pixel-level (`set_pixel` is
  the primitive; higher-level shapes are a future change if needed).
- No variable-width fonts, word-wrap, or multi-line layout helpers.
- No DMA/non-blocking I2C (still blocking `HAL_I2C_Mem_Write`, consistent
  with `ssd1309-display-bringup`).

## Decisions

- **Framebuffer lives in `ssd1309_t`, fixed-size `uint8_t buffer[1024]`.**
  1024 bytes = 128 cols x 8 pages, the max size for this controller (128x64).
  For panels smaller than 128x64 (via `ssd1309_init()`'s `width`/`height`),
  only the first `width * (height/8)` bytes are used. A fixed array avoids
  dynamic allocation (no heap use in this codebase) and 1024 bytes is
  trivial against the F767's 512KB SRAM. Alternative considered: separate
  `ssd1309_fb_t` struct passed alongside `ssd1309_t` — rejected as
  unnecessary indirection for a single-display sandbox driver.

- **Pixel addressing**: `byte_index = (y / 8) * p->width + x`,
  `bit = y % 8`, bit value 1 = pixel on. This matches the GDDRAM
  page-major/LSB-top layout so `ssd1309_flush()` can write
  `buffer[page*width .. page*width+width-1]` directly as one page's data,
  identical in shape to the existing test-pattern write.

- **`ssd1309_set_pixel(p, x, y, color)` / `ssd1309_get_pixel(p, x, y)`
  bounds-check against `p->width`/`p->height` and silently no-op / return 0
  out of range.** Keeps callers (especially `draw_char`/`draw_string`) simple
  — text that runs off the edge is clipped rather than corrupting adjacent
  rows or requiring callers to pre-check.

- **`ssd1309_clear(p)` = `memset(buffer, 0x00, ...)`,
  `ssd1309_fill(p, color)` sets every byte to `0x00` or `0xFF`.** Only
  whole-byte (whole-page-row) fills are supported via `fill`; arbitrary
  rectangle fills are out of scope (Non-Goals).

- **Whole-buffer `dirty` flag, set only on an actual content change.**
  `ssd1309_t` gains a `bool dirty` field, initialized to `false` by
  `ssd1309_init()`.
  - `ssd1309_set_pixel()` compares the target bit's current value to
    `color` before writing; if they differ, it updates the bit and sets
    `dirty = true`. If the pixel is already the requested color (including
    all out-of-bounds/no-op calls), `dirty` is left unchanged.
  - `ssd1309_clear()` and `ssd1309_fill()` always set `dirty = true`
    unconditionally — checking whether a bulk operation actually changed
    anything would mean comparing the whole buffer first, which costs as
    much as just flushing it, so it's not worth optimizing.
  - `ssd1309_flush()` checks `dirty` first: if `false`, it returns `HAL_OK`
    immediately without any I2C traffic. If `true`, it performs the full
    page-write loop as normal and, only on success, sets `dirty = false`.
    On I2C failure, `dirty` remains `true` so a later `flush()` retries the
    write.
  This gives "flush is a no-op until something actually changed" cheaply,
  without per-page bookkeeping — `app_loop()` can call `ssd1309_flush()`
  on every `flag_get_100MS()` tick and it'll only hit the bus when the
  on-screen content has actually changed.

- **Shared internal page-write helper.** Extract a static
  `ssd1309_write_pages(ssd1309_t *p, const uint8_t *data)` that performs the
  per-page `0xB0+page / 0x00 / 0x10` column-reset + data write loop
  currently inlined in `ssd1309_bringup()`'s test-pattern step.
  `ssd1309_bringup()` calls it with a local all-`0xFF` buffer (preserving its
  documented behavior exactly — no spec change), and the new
  `ssd1309_flush()` calls it with `p->buffer`. This is an internal
  refactor only; `ssd1309-display-bringup`'s observable behavior and spec
  are unchanged.

- **Font: fixed 5x7 glyph bitmaps in a 6px-wide cell (5px glyph + 1px
  spacing), 8px tall, covering printable ASCII `0x20`-`0x7E` (95 glyphs).**
  Stored as `static const uint8_t font5x7[95][5]`, one byte per column,
  bit 0 = top row — i.e. each glyph column maps directly onto one
  framebuffer byte's bit layout, so `draw_char` is a tight nested loop over
  `set_pixel`. This is the same widely-used 5x7 layout found in most
  SSD1306/SSD1309 Arduino-style libraries; covering only printable ASCII
  keeps the table small (475 bytes) and sufficient for debug status text.

- **`ssd1309_draw_char(p, x, y, c, color)` and
  `ssd1309_draw_string(p, x, y, str, color)`.** `draw_string` advances `x`
  by 6px per character (no wrapping); characters outside `0x20`-`0x7E` are
  rendered as a blank cell. `color` selects on/off so text can be drawn in
  either polarity (e.g. for inverted highlight blocks later).

- **When `dirty` is true, `ssd1309_flush()` writes the full 1024-byte (or
  `width*height/8`-byte) buffer in one pass over 8 pages** — no per-page
  dirty tracking, just the single whole-buffer flag above. Simpler and
  sufficient for a ~128x64 debug display updated at human-visible rates.

## Risks / Trade-offs

- **Full-buffer flush cost (when dirty)**: 8 page-select writes (3 bytes
  each) + 8 data writes (128 bytes each) per `ssd1309_flush()`, all blocking
  I2C at 400 kHz → roughly ~1100 bytes of I2C traffic per flush
  (~25-30 ms at 400 kHz including per-byte ACK overhead) → Mitigation: the
  whole-buffer dirty flag means this cost is only paid when content
  actually changed; a future change could add per-page dirty tracking or
  DMA if higher refresh rates are needed.
- **Font table size/copyright**: using a "standard" 5x7 font layout that's
  ubiquitous in open-source SSD1306/1309 libraries → Mitigation: write the
  glyph table from scratch as a fresh bitmap (not copy-pasted from a
  specific library file), so no licensing question arises.
- **No bounds-checking cost on hot path**: `set_pixel`/`get_pixel` bounds
  checks add a branch per pixel → negligible at this resolution/refresh
  rate; not optimized further.

## Open Questions

- None — scope confirmed (framebuffer + primitives + flush + basic text,
  full-buffer flush only).
