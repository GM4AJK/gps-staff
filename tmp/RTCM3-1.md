
# RTCM3 handling specification

This spec outlines a module called rtcm3 (rtcm3.h/.c) and shows how data "comes in" and goes out OTA.

We assume an RTCM3 message has a maximum length of 1024 bytes.
We reserver enough SRAM for 4 complete messages and maintain an index int as to which is "in use for rx" and which is "in use for tx"
If all four buffers are full we drop any incoming RTCM3 frames from the serial port until one of the four buffers is available.

1. Data arrives on a UART type serial port.

I expect a function called rtcm3_uart_in_irq() that executes within the STM32's interrupt context.

The user of the RTCM3 module is expected to connect up the CubeMX IRQ handler to call rtcm3_uart_in_irq() when a char is rxed on the UART.

When a char arrives on the serial port rtcm3_uart_in_irq() should place it into the currently "in use for rx" buffer.

rtcm3_uart_in_irq() should detect the RTCM3 start frame char 0xD3 to align itself, then collect the next 2 bytes to determine the frame length and then
begin filling the current input buffer (including the leading 0xD3 and length in that buffer).

When all frame bytes have been read rtcm3_uart_in_irq() sets a flag for the idle loop to capture.

2. I expect the rtcm3 module to export an rtcm3_loop() function that is called repeatedly from the idle loop.

When rtcm3_loop() detects rtcm3_uart_in_irq() setting a flag to indicate a frame has been received it should send a message to teh debug UART saying "RTCM3 message received\r\n"

This is my requirement for READING RTCM3 data from a UART. Once complete we will move on to teh next stage of chucnking it and sending OTA.





