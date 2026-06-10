## 1. Driver handle rework

- [x] 1.1 Change `ssd1309_t.port` from `I2C_HandleTypeDef` to
      `I2C_HandleTypeDef *` in `Core/Inc/ssd1309.h`
- [x] 1.2 Update `ssd1309_init()` signature to take `I2C_HandleTypeDef *in_port`
      and store it directly (no I2C traffic), update `Core/Src/ssd1309.c`
      accordingly

## 2. Command/data write helpers

- [x] 2.1 Add `#define`/enum constants for the SSD1309 command opcodes used
      in the bring-up sequence (display off/on, clock divide, multiplex
      ratio, display offset, start line, segment re-map, COM scan
      direction, COM pins config, contrast, pre-charge, VCOMH, entire
      display on, normal display, deactivate scroll, memory addressing
      mode, page/column address)
- [x] 2.2 Add a static helper to send a command byte sequence via
      `HAL_I2C_Mem_Write` with control byte `0x00`
- [x] 2.3 Add a static helper to send a GDDRAM data byte sequence via
      `HAL_I2C_Mem_Write` with control byte `0x40`

## 3. Bring-up sequence

- [x] 3.1 Implement `HAL_StatusTypeDef ssd1309_bringup(ssd1309_t *p)` in
      `Core/Src/ssd1309.c`: send the power-on init command sequence from
      `design.md` (multiplex ratio derived from `p->height`), checking and
      propagating `HAL_StatusTypeDef` from each transfer
- [x] 3.2 In `ssd1309_bringup()`, after init commands succeed, write the
      all-`0xFF` test pattern to all 8 pages x 128 columns via
      page-addressing mode
- [x] 3.3 Send `0xAF` (display on) as the final step and return `HAL_OK`
- [x] 3.4 Declare `ssd1309_bringup()` in `Core/Inc/ssd1309.h`

## 4. Sandbox integration

- [x] 4.1 In `Core/Src/app.c`, add a static `ssd1309_t oled` handle
- [x] 4.2 In `app_init()`, call `ssd1309_init(&oled, &hi2c1, 0x3C, -1, -1)`
      followed by `ssd1309_bringup(&oled)`
- [x] 4.3 On `HAL_StatusTypeDef` error from `ssd1309_bringup()`, print a
      diagnostic message over USART3 (matching the existing
      `HAL_UART_Transmit` heartbeat style in `app_loop()`)

## 5. Bench verification

- [x] 5.1 Build and flash via the external ST-LINK V3, confirm the OLED
      panel lights fully (all pixels on) on boot
- [x] 5.2 If the panel does not light: verify I2C address (`0x3C` vs
      `0x3D`), check SDA/SCL wiring on PB8/PB9, and check the module's RES#
      strap per `design.md` Non-Goals/Risks
