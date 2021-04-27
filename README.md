# Introducing the DUCC

The Data-logger Utility for Citizen Collaboration (DUCC) is a nascent datalogging platform aiming to streamline the sensor-to-database pipeline for grassroots water quality advocates and citizen scientists.

This first iteration of the design was born of the ECE4872 Senior Design course at The Georgia Institute of technology in Spring 2021. It is a low-cost and beginner-friendly embedded sensing unit that can be used to capture four data points (pH, turbidity, temperature, and TDS) to monitor water quality. 

This is an initial proof of concept, and further development is needed before this device is ready widespread use. Specific areas that need attention include the power supply, integration with Adafruit.io, and sensor calibration/verification. Additionally, the use of an alternative microcontroller should be explored as the Adafruit Huzzah32 has proven to be less compatible with this application than expected.

Watch a short demonstration video [here](https://www.youtube.com/watch?v=ByALw4eoFNU&feature=youtu.be)

# Build Your Own DUCC
Contents
- [Parts List](#Parts-List)
- [Step 1: Hardware Assembly](#Assembly-Guide)
- [Step 2: Program the DUCC](#Software-Guide)
- [Step 3: Deploy](#Deployment-Guide)

## Parts List
| Component  | Cost | Notes |
| ------------- | ------------- | ------------- |
| [Adafruit HUZZAH32 – ESP32 Feather Board](https://www.adafruit.com/product/3591) | $21.95 | Be sure to order the version with **stacking headers**! |
| [Adafruit 2500 mAh LiPo Battery](https://www.adafruit.com/product/328) | $14.95 | Alternatively use the [4400 mAh version](https://www.adafruit.com/product/354) for $19.95 |
| [DS18B20 Sensor Kit](https://www.dfrobot.com/product-1354.html)  | $7.50 | Be sure to order the sensor **kit**, not the standalone sensor! |
| [Turbidity Sensor ](https://www.dfrobot.com/product-1394.html)  |  $9.90 |  |
| [TDS Sensor](https://www.dfrobot.com/product-1662.html)  | $11.80 |  |
| [pH Sensor](https://www.dfrobot.com/product-1025.html)  | $29.50 |  |
| [F-F Jumper Wires](https://www.adafruit.com/product/3633)  | $2.95 |  |
| [Boost Converter](https://www.amazon.com/gp/product/B08DNS3R1Y/ref=ppx_yo_dt_b_asin_title_o05_s00?ie=UTF8&psc=1)  | $6.99 |  |
| [Mini Breadboards](https://www.amazon.com/gp/product/B0146MGBWI/ref=ppx_yo_dt_b_asin_title_o03_s00?ie=UTF8&psc=1)  | $6.98 |  |

## Assembly Guide
1. Identify the 3V and GND pins on your Adafruit Huzzah-32 Feather board. Plug one jumper wire into each. 
2. Plug the other end of each jumper wire into a miniature breadboard.


## Software Guide
### 1. Download the Arduino IDE
  - Download and install the [Arduino IDE](https://www.arduino.cc/en/software) from the Arduino website **NOT the Windows App**.
  - Install the ESP32 using the IDE’s [board manager](https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md).
  - Install the sketch data uploader extension.
### 2. Download the DUCC Embedded App
  - Download this repository and move the ```embedded_app``` folder into your local Arduino folder. Open embedded_app/embedded_app.ino in the Arduino IDE.
### 3. Install the Sensor Library
  - Follow the instructions [here](https://github.com/TheMightyDUCCs/sensors).
### 4. Install Bluefruit Connect
  - Install the [Bluefruit Connect](https://play.google.com/store/apps/details?id=com.adafruit.bluefruit.le.connect&hl=en_US&gl=US) app on your phone or orther personal device.

## Deployment Guide

TODO: Include instructions on potential multi-node configurations for up-stream down-stream experiments and ideas for applications.
