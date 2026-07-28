## Nova
# Your pocket assistant

A pocket assistant built from three parts and wired by hand: a microcontroller, a touchscreen, and a camera — talking to AI over Wi-Fi to answer whatever you type or point it at.

## Connections

| TFT Module Pin | ESP32-S3 GPIO | Description |
|----------------|---------------|-------------|
| VCC | 3.3V | Display Power |
| GND | GND | Ground |
| CS | GPIO21 | TFT Chip Select |
| RESET | GPIO2 | TFT Reset |
| DC / RS | GPIO14 | Data / Command |
| SDI (MOSI) | GPIO42 | SPI MOSI |
| SCK | GPIO41 | SPI Clock |
| LED | 3.3V | Backlight Power |
| SDO (MISO) | GPIO47 | SPI MISO |
| T_CLK | GPIO41 | Touch SPI Clock (Shared) |
| T_CS | GPIO1 | Touch Chip Select |
| T_DIN | GPIO42 | Touch SPI MOSI (Shared) |
| T_DO | GPIO47 | Touch SPI MISO (Shared) |
| T_IRQ | Not Connected *(Optional GPIO40)* | Touch Interrupt |

**There is no wiring required for camera module. It will be directly connected to Esp32 board**