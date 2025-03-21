# STM32F407 with TFT LCD and MQTT

The two projects in this repository illustrate how one could design a standalone product using an STM32F407 Discovery Kit. The documents, `STM32F407 Discovery Kit with TFT LCD Display.pdf` and `STM32F407 Discovery Kit with ESP32 and MQTT.pdf` provide detailed instructions for their respective projects.

The `tft_lcd` project connects the STM32F407 with a touchscreen display and demonstrates communication back and forth between them. The `tft_lcd_mqtt` project expands on this to incorporate wireless communicaton via the MQTT protocol, commonly used in Internet of Things (IoT) applications.

The completed STM32 projects can be found in their corresponding folders. To provide WiFi connectivity to the STM32F407 for the `tft_lcd_mqtt` project, an ESP32-C3-DevKitC-02 is used, running `mqtt_json_uart.ino`.

If you are new to the STM32F407 or STM32CubeIDE, check out the [STM32F407 Discovery Kit Familiarization](https://github.com/zvdv/STM32F407-Discovery-Kit-Familiarization) for introductory projects and tips.
