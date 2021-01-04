# M5Stack Core2 for AWS IoT EduKit Code Repository
This is the accompanying code repository for microcontroller tutorials presented in the [AWS IoT EduKit](https://edukit.workshop.aws) program using the [M5Stack Core2 for AWS IoT EduKit](https://m5stack.com/products/m5stack-core2-esp32-iot-development-kit-for-aws-iot-edukit) reference Hardware.

Each of the folders in this repository contains a separate project as described below. All projects require the **ESP-IDF 4.2**, except for the project in the `Getting-Started` folder, which uses PlatformIO and their Espressif 2.1.0 platform configuration. Please ensure that the ESP-IDF v4.2 is updated and added to your path first. Follow the [AWS IoT EduKit — Blinky Hello World](https://edukit.workshop.aws/en/blinky-hello-world.html) tutorial for instructions on how to setup your environment.

For Arduino, UIFlow, or MicroPython content and code, please view the official [M5Stack Docs](https://docs.m5stack.com/#/).

## Included Projects
### Core2 for AWS IoT Features Demo
This project is a demo of the hardware features available on the M5Stack Core2 for AWS IoT EduKit reference hardware. It uses at least one available API of each hardware feature in the board support package (BSP). The BSP drivers are located in the **/components/core2forAWS/** directory. There is also a ported version of Espressif's ESP-CRYPTOAUTHLIB to be used with the BSP for the Microchip ATECC608 Trust&GO secure element to function.

You can flash this firmware by entering the command into your shell prompt (replace **<<DEVICE_PORT>>** with the serial port your Core2 for AWS IoT EduKit device is connected to):
```bash
cd Core2-for-AWS-IoT-EduKit-Features-Demo
idf.py build -p <<DEVICE_PORT>>
```

## Core2 for AWS  IoT EduKit Factory Firmware
This project is the factory firmware that comes loaded with the device. It contains basic functionality and can be used to restore the device to factory state by running the following command (replace **<<DEVICE_PORT>>** with the serial port your Core2 for AWS IoT EduKit device is connected to):
```bash
cd Core2-for-AWS-IoT-EduKit-Factory-Firmware
idf.py build erase_flash flash -p <<DEVICE_PORT>> 
```

### Getting Started
This project is used in the [AWS IoT EduKit — Getting started](https://edukit.workshop.aws/en/getting-started.html) tutorial. It contains a port of [ESP RainMaker](https://rainmaker.espressif.com/) and uses [PlatformIO](https://platformio.org/). It is a quick end-to-end demonstration of a smart home application. Please follow the tutorial for usage.

### Blinky Hello World
This project is used in the [AWS IoT EduKit — Blinky Hello World](https://edukit.workshop.aws/en/blinky-hello-world.html) tutorial. It is a basic blinky LED demo that uses the on-board secure element for provisioning the device to AWS IoT. This example uses the AWS IoT Device SDK for Embedded C and the ESP-IDF v4.2. Please follow the tutorial for usage.

### Smart Thermostat
This project is used in the [AWS IoT EduKit — Smart Thermostat](https://edukit.workshop.aws/en/smart-thermostat.html) and [AWS IoT EduKit — Smart Spaces](https://edukit.workshop.aws/en/smart-spaces.html) tutorials. Is a demonstration that uses AWS IoT device shadows. This example uses the AWS IoT Device SDK for Embedded C and the ESP-IDF v4.2. Please follow the tutorial for usage.

### Alexa for IoT-Intro (Beta)
This project is used in the [AWS IoT EduKit — Intro to Alexa for IoT](https://edukit.workshop.aws/en/intro-to-alexa-for-iot.html) tutorial. It contains several Alexa for AWS IoT (AIA) features including english "Alexa" wake word detection, smart home device, audio player and others. Please follow the tutorial for usage. This is preview software based on the [ESP-VA-SDK](https://github.com/espressif/esp-va-sdk), and is not a stable port.

## Support
For issues with the AWS IoT EduKit content or this repo, please [submit an issue](https://github.com/m5stack/Core2-for-AWS-IoT-EduKit/issues) to this repository.