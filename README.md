# ESP32-S3 Digit Recognition in LVGL

## Development Timeline

| Date       | Milestone/Note   |
|------------|------------------|
| 2025-09-11 | Project started  |
| 2025-09-12 | Add Imporvement: Larger input size, auto predic |

## Introduction

This project demonstrates digit recognition on an ESP32-S3 development board. It is based on [touchpad_digit_recognition](https://github.com/espressif/esp-iot-solution/tree/master/examples/ai/esp_dl/touchpad_digit_recognition), and integrates the [LVGL library](https://github.com/tommokmok/esp32s3_lvgl_idf) and the [Sketchpad application](https://github.com/tommokmok/esp32s3_YetAnotherSketchpad).

It combines my previous work on ESP-DL and LVGL porting. The goal is to allow users to draw a digit on the screen, then have the device predict the number using the esp-dl library.

## Hardware

- **ESP32-S3 Development Board** with 2.8" TFT LCD  
  - Purchased from Taobao. More info: [ESP32-LVGL开发板](http://wiki.waaax.cn/boards/ESP32/ESP32-LVGL%E5%BC%80%E5%8F%91%E6%9D%BF.html)
  > **Note:** This dev board is not recommended due to hardware issues. It may reset automatically, especially when connected to a PC.
  - **ESP32-S3-N8R8** (8MB external RAM, 8MB external SPI flash)
  - **LCD Driver:** ST7789, 320x240 pixels (SPI interface)
  - **Touch Driver:** XPT2046 (SPI interface)

## Software Dependencies

- **ESP-IDF:** v5.4.2
- **esp_lvgl_port:** v2.6.0 (ESP Component Registry)
  - Depends on **LVGL:** v9.2.2
- **atanisoft__esp_lcd_touch_xpt2046:** v1.0.6 (ESP Component Registry)
  - Depends on **esp_lcd_touch:** v1.1.2
- **esp-dl:** v3.1.3 (ESP Component Registry)
  - Depends on **esp_new_jpeg:** v0.6.1

## How This Project May Help You

- Learn how to bridge UI and deep learning on ESP32-S3.
- See practical integration of LVGL and ESP-DL for digit recognition.

For detailed implementation, please check my blog: [https://tommokmok.github.io/](https://tommokmok.github.io/)

## Features

- Simple sketchpad for drawing digits on the screen
- Button to clear the screen
- Button to send input data to the neural network and update the recognized digit text

## Screenshots

<a href="https://ibb.co/bMy95SGs"><img src="https://i.ibb.co/bMy95SGs/lvgl-digit-reconigition.jpg" alt="lvgl-digit-reconigition" border="0"></a>

## Test Video

[Please check](https://www.youtube.com/shorts/gx1x1HtLqbE)



## Prerequisites

- Visual Studio Code with Espressif IDF extension
- ESP-IDF configured to version v5.4.2

## Building & Flashing

1. **Clone this repository.**
2. **Open the project in VS Code.**
3. **Set the device target to `esp32s3`** in the IDF extension.
4. **Click `Build` and `Flash`.**

## Implementation Notes

- The original `touchpad_digit_recognition` used a touchpad for input; I have uncommented all related code for this project.
- I implemented a component `lvgl_ui` for all LVGL UI updates, exposing an API to update the prediction text.
- In `main`, I inject the function `touch_digit_send_to_dl` to allow LVGL to send input data to the ESP-DL task.

## Improvements

- The input size is currently too small. Consider increasing it from 64x64 to 128x128. -- Done
- When clearing the screen, the output text should also reset to an empty state. -- Done
- Can implment a real time digit recongnition -- Done (Set to 200ms delay)
