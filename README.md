# bleskomat-device

The Lightning Network ATM with simple components and a simple setup - just plug it in and it works!

* [Overview](#overview)
* [Requirements](#requirements)
	* [Hardware Requirements](#hardware-requirements)
	* [Software Requirements](#software-requirements)
* [Setup](#setup)
	* [Building the Hardware Device](#building-the-hardware-device)
	* [Cable map to connect ESP32 to TFT screen](#cable-map-to-connect-esp32=to-tft-screen)
	* [Installing Libraries and Dependencies](#installing-libraries-and-dependencies)
	* [Generating Your Local Config File](#generating-your-local-config-file)
	* [Compiling and Uploading to Device](#compiling-and-uploading-to-device)
* [License](#license)


## Overview

Key features include:
* Works offline - no WiFi required
* Inexpensive, easily-sourced components
* Easily hackable and extendible

The project consists of two parts:
* __Physical Device (ATM)__ - user inserts coins, device generates a signed URL and displays as QR code, user's app (which supports lnurl-withdraw) scans QR code and makes request to HTTP server, withdraw process is completed and the user has successfully bought satoshis with fiat coins.
* __HTTP Server__ - supports [lnurl-withdraw](https://github.com/btcontract/lnurl-rfc/blob/master/lnurl-withdraw.md) with additional request handlers for fiat-currency -> satoshi conversion and request signing.

This repository contains the source and build instructions for the physical device. The source code and documentation for the HTTP server component is located in a [separate repository](https://github.com/samotari/bleskomat-server).


## Requirements

This section includes information about the requirements (software + hardware) that you will need to build the physical Bleskomat ATM. 


### Hardware Requirements

To build the physical device, you will need the following hardware components:
* [ESP-WROOM-32](https://www.espressif.com/en/products/modules/esp-wroom-32/overview) by espressif
* TFT Display
* Coin Acceptor - "Model HX-616"
* 12V DC power adaptor
* USB car charger (12V -> 5V DC)


### Software Requirements

* [PlatformIO Core (CLI)](https://docs.platformio.org/en/latest/core/)
* [nodejs](https://nodejs.org/) - For Linux and Mac install node via [nvm](https://github.com/creationix/nvm)


## Setup

Step-by-step setup process including both hardware and software.

### Building the Hardware Device

Before proceeding, be sure that you have all the project's [hardware requirements](#hardware-requirements).

!! TODO !! Write detailed instructions (w/ pictures) to build the device.

### Cable map to connect ESP32 to TFT screen

|  ESP32       | TFT      |
|--------------|----------|
| VIN          | VCC      |
| GND          | GND      |
| GPIO5  (D5)  | CS       |
| GPIO16 (RX2) | RS       |
| GPIO17 (TX2) | AO (DC)  |
| GPIO23 (D23) | SDA      |
| GPIO18 (D18) | SCK      |
| 3.3V (3V3)   | LED (NC) |


### Installing Libraries and Dependencies

Before proceeding, be sure that you have all the project's [software requirements](#software-requirements).

First thing to do is to install npm dependencies:
```bash
npm install
```
Node is used as a task runner and to generate new (or read existing) API keys.

Next you will need to install required libraries for the C/C++ builds:
```bash
platformio lib install
```
See the `platformio.ini` file for a list of libraries that will be downloaded and installed.

If while developing you need to install a new library, use the following command as a guide:
```bash
platformio lib install --save LIBRARY_NAME[@VERSION]
```
The `--save` flag tells platformio to add the library to the project's `platformio.ini` file.

You can find PlatformIO's libraries repository [here](https://platformio.org/lib).


### Generating Your Local Config File

A helper script is used to generate the local configuration file that is needed to connect to the server's Postgres database, encrypt/decrypt sensitive data (like API key secrets), and to set other required configuration options for the device build process. Let's get started:
```bash
npm run config -- load
```
This will walk you thru the process of generating the local config file.

You can choose to generate an encrypted or decrypted config file. You should note that the build process will require an unencrypted config file in order to run. It is possible to decrypt the config file as follows:
```bash
npm run config -- decrypt
```
Or to encrypt it:
```bash
npm run config -- encrypt
```


### Compiling and Uploading to Device

To compile the project (without uploading to a device):
```bash
API_KEY_ID="XXX" npm run compile:only
```
To run the build with dummy/invalid API-key-related build flags:
```bash
API_KEY_NONE=1 npm run compile:only
```

To compile and upload to your device:
```bash
API_KEY_ID="XXX" npm run compile:upload
```
If you receive a "Permission denied" error about `/dev/ttyUSB0` then you will need to set permissions for that file on your system:
```bash
sudo chown $USER:$USER /dev/ttyUSB0
```
To run the build with dummy/invalid API-key-related build flags:
```bash
API_KEY_NONE=1 npm run compile:upload
```

To open the serial monitor:
```bash
npm run monitor
```



## License

This project is "unlicensed" meaning all copyrights are withheld. This is the closed-source, extended version of the device for the commercial Bleskomat ATMs.

There is an [open-source version](https://github.com/samotari/bleskomat) of this project that hackers and hobbyist are welcome to use as a guide to build their own Bleskomat ATMs.
