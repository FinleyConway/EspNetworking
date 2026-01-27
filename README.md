# EspNetworking
This project implements a server-client system, where a PC server connects to and communicates with multiple ESP32 clients simultaneously. It uses [ASIO](https://github.com/chriskohlhoff/asio) and FreeRTOS for asynchronous networking, enabling reliable, non-blocking communication between devices.

## Building
### Client
Uses VSCode PlatformIO IDE extension to build, flash and monitor esp32s.

#### TODO: Be able to flash wifi informations and other important information
#### TODO: Look into using the tools provided by ESP-IDF so PlatformIO isn't a requirement 

### Server

Depends only on `libncurses` for gui

#### TODO: Perhaps add a server-headless that doesnt rely on imtui/imgui?

### Linux and Mac:
```
git clone https://github.com/FinleyConway/EspNetworking.git --recursive
cd EspNetworking
cmake -S . -B build && cmake --build build

./build/esp_networking
```
### Windows:
```
```

## Running
Currently uses mDNS to allow the esp32 to discover the server using the server devices' hostname. However currently the expected hostname is 'fedora.local' but im sure it's possible to use mDNS on a custom hostname.

#### TODO: Set a better expected hostname and instructions to set custom hostname without changing system hostname

### Linux:
Using avahi to enable mDNS
```
sudo systemctl start avahi-daemon.socket avahi-daemon.service
```
### Mac:
```
```

### Windows:
```
```

--- 

When esp_networking is running, resetting/turning on esp32 will attempt to connect to TCP server. Alternative, connecting a push button do pin13 allows you reset the esp allowing to connect/disconnect from the server smoothly. 

Under Conenct tab you will see all connected esp32.
<img width="2050" height="1166" alt="image" src="https://github.com/user-attachments/assets/8eace00b-c7be-48dc-886c-178473ea0988" />
