# RacingCar

## Introduction

This is a project about CUHK engine jyunhing 2022 (香港中文大學工程學院學生慶祝活動2022).  
The instagram page is [@22engine_jyunhing](https://www.instagram.com/22engine_jyunhing/)  
This project aims to build a remote racing car with 2 ESP-32 and controlled by handmade controller via WIFI.  

## Pre-requisite

- Arduino IDE v2
- Hardware to be added

## Environment

- This project is done under Arduino IDE == 2.0.4, but Arduino IDE >= 2.0.0 should be fine to run this project.

- Install library for ESP32
  1. In your Arduino IDE, go to `File > Preferences`
  2. Paste the link below into `Additional Board Manager URLs` field:
   ``
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ``
   ![Preference](./image/Preference.png)
  3. Then go to `Tools > Board > Boards Manager`
  4. Search esp and install required library (There should only be one result)
  ![step1](./image/step1.png)

- Select the correct board module, We use ESP32-WROOM-DA for this project.
  ![select board](image/select.png)
  ![step2](./image/step2.png)

## Hardware connection

WIP
