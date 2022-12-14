# RacingCar

## Introduction

This is a project about CUHK engine jyunhing 2022 (香港中文大學工程學院學生慶祝活動2022). Instagram page: [@22engine_jyunhing](https://www.instagram.com/22engine_jyunhing/)  
This project aims to build a remote racing car with 2 ESP-32 and controlled by handmade controller via WIFI.  

## Component

- Arduino IDE v2
- Motor x 2
- ESP32(ICC: ) x 2
- Motor Driver(Model: TB6612FNG) x 1
- Joystick x 1
- Button x 2

## Environment

- The code is written under linux (Arch 6.0.2), but Windows 10/11 and MacOS should also work.
- This project is done under Arduino IDE == 2.0.1, but Arduino IDE >= 2.0.0 should be fine to run this project.
- Install Arduino IDE and driver (CP210) from [offical website](https://www.arduino.cc/en/software).
- Install library for ESP32, this project is done with library version == 2.0.5
  1. In your Arduino IDE, go to `File > Preferences`
  2. Paste the link below into `Additional Board Manager URLs` field:
   ``
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ``
   ![Preference](./image/Preference.png)
  3. Then go to `Tools > Board > Boards Manager`
  4. Search esp32 and install required library (There should only be one result)
  ![step1](./image/step1.png)

- Select the correct board module, We use `ESP32 Dev Module` for this project.
  ![select board](image/select.png)
  ![select ESP32 DEV Module](./image/board.png)

## Hardware connection

### Receiver (Car)

| Source | Destination  |
|--------|--------------|
| PWMA   | ESP32 25     |
| AIN2   | ESP32 27     |
| AIN1   | ESP32 26     |
| STBY   | ESP32 33/VCC |
| BIN1   | ESP32 16     |
| BIN2   | ESP32 17     |
| PWMB   | ESP32 5      |
| VM     | Car Bat +    |
| GND    | Car Bat -    |
| AO1    | Front Motor  |
| AO2    | Front Motor  |
| BO2    | Back Motor   |
| BO1    | Back Motor   |
| GND    | ESP32 GND    |

![connection graph](./image/motorDriver.png)

### Sender (Controller)  

| Source       | Destination |
|--------------|-------------|
| Joystick GND | ESP32 GND   |
| Joystick VCC | ESP32 32    |
| Joystick VRx | ESP32 33    |
| ButA GND     | ESP32 GND   |
| ButA VCC     | ESP32 14    |
| ButA OUT     | ESP32 12    |
| ButB GND     | ESP32 GND   |
| ButB VCC     | ESP32 26    |
| ButB OUT     | ESP32 27    |

![connection graph](./image/controller.png)
Note: Connect to Joystick VRy instead of VRx depends on the joystick.

## Code

1. Flash the receiver code into car esp32.
2. Serial Monitor(115200 baud) should show the local MAC address
![upload  receiver](./image/receiver.png)
3. Change MAC Address in sender code to that MAC address shown
![upload sender](./image/sender.png)
4. FLash the sender code into controller esp32.

After 4 step above, the car should work. If not working, Trouble shooting section below may help.

## Troubleshooting

### Driver

For linux, you may need to

1. Install `pyserial`  
``pip install pyserial``
2. Add to `uucp` group  
``usermod -aG uucp <user_name>``
3. Load `cdc_acm` module  
``modprobe cdc_acm``  

Note: Step 2 may need reboot to take effect.

### No connection established

Make sure the board is turn on, and `lsusb` to see whether the cable using can transmit data (The esp32 board should shown).

Try to uninstall esp32 library from board manager and reinstall, some Arduino IDE version having this bug arduino/arduino-cli#1970.

### Debug procedure

Follow the flow below, if encounter unknown issue, it would probably hardware issue, try to replace a new one.
![debug procedure](./image/debug.png)

### Wrong boot mode detected

If when uploading, Arduino IDE show wrong boot mode detected error (exit status 2). Try upload again and when `Connecting...` shown, hold the Boot button on ESP32
![wrong boot mode](./image/exitcode2.png)
