## Nova
# Your pocket assistant

A pocket assistant built from three parts and wired by hand: a microcontroller, a touchscreen, and a camera — talking to AI over Wi-Fi to answer whatever you type or point it at.

<img width="1920" height="1080" alt="Screenshot (572)" src="https://github.com/user-attachments/assets/5373a77d-a20f-4307-b5e9-11b7209b10c2" />
<img width="1920" height="1080" alt="Screenshot (576)" src="https://github.com/user-attachments/assets/37784f2f-3057-4dd1-8eee-58168c5ac678" />
<img width="1920" height="1080" alt="Screenshot (575)" src="https://github.com/user-attachments/assets/6cc45ced-ce0e-4366-abe3-c02f83434ca8" />
<img width="3508" height="2480" alt="image" src="https://github.com/user-attachments/assets/d3c838c4-ffd6-41ad-a1a2-930430f46e19" />




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

## Firmware

void setup ():- Starts the shared SPI bus, initializes the screen and touch controller, connects to Wi-Fi, starts the camera, then draws ai bot UI.

Void loop():- Checks for touches in touch screen.

Rest are functions for each tasks like connecting to wifi, initialising camera takes photos etc.
