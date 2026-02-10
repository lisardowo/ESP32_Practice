# H>E>R>M>E>S 

Hermes is a "doomsday" communication device that uses the LoRa protocol to send messages via low powered, long-ranged signals that allow encrypted communications off the grid without dependency in internet providers or bluetooth protocols

## Developed by:

Lisardo Sobrado Morales, Diego del Rio Pulido, Jorge Alberto Rivera Ahumada 

## Overview

Hermes is a phone-like device that uses LoRa signals to send encrypted messages to its fellow models without using any modern digital infrastructure as wifi or bluetooth ensuring its reliability amongst the worst scenarios. 
Hermes is designed as the first part in a several "doomsday - devices" project, being the hub that will connect all future modules :

- Encrypts message for secure communication
- Off the grid connection
- Portability
- Foccus in privacy and security
- Central Part of a transversal "judgement day" inspired devices

## Features

- **Messaging off the grid**
- **Encrypted Communications**
- **Modular Design**


## Project Structure

```
Hermes
├─ CMakeLists.txt
├─ LICENSE
├─ README.md
├─ esp32.code-workspace
├─ iliespWiring.png
├─ include
│  ├─ README
│  └─ ili9341.h
├─ lib
│  └─ README
├─ platformio.ini
├─ sdkconfig.esp32doit-devkit-v1
├─ src
│  ├─ CMakeLists.txt
│  ├─ ili9341.c
│  ├─ main.c
│  ├─ setup
│  └─ transmiter.c
├─ test
│  └─ README
└─ testplayground
   └─ transmiter

```

## Prerequisites

- **Work in progress**

## Installation

### work in progress

## Usage

### work in progress

## Technologies Used

### Software

- **C as primary languague** 
- **Platform.io**

### Hardware

- **Esp32 microcontroller**
- **LoRa SX1278 module**
- **Display ili9341**

## Development

The project uses:

- C as the main language
- HandMade libraries for all modules (no external libraries)


## Contributing

This project was built by the yakucode team, for the people. Contributions are welcome! Feel free to:

- Report bugs
- Suggest new features
- Submit pull requests
- Improve documentation
- Refactorize and optimize bad code

For any contributions please refeer to the [CONTRIBUTING.md](CONTRIBUTING.md)

## License

The project is published under the GPL license, this is an open source project available for educational and investigation purposes.

### This device is intended solely for experimental purposes and is not meant for any illegal activities. We do not condone unlawful behavior and strongly encourage you to use it only within the bounds of the law. 

## ROADMAP

To make HERMES a production-ready system, these are some basic features we SHOULD have:

### Hardware

- [ ] Creating the device :
  
   - [ ] Designing a pcb —  Profesional device design
   - [ ] Make it modular — Specially manufactured for coupling with other modules
   - [ ] Prototype :
        - [X] esp32 to ili
        - [ ] esp32 to LoRa
- [ ] Make case — phone-like design 
- [ ] Design a full UI :
    - [ ] MENU - messages, extensions(via modules), device information
    - [ ] Message Menu - Coneccted devices
    - [ ] Extension Menu - select ampliation, use that module
    - [ ] Device Menu - Device information, battery, ID, Session Time, host name, version 
    - [ ] Cute animations - opening a menu, sending message, processing, loading




### Software

   - [ ] Headers : 
    
        - [X] ili9341.h
        - [ ] sx1278.h
        - [X] transmission.h
        - [ ] modules.h
        - [ ] menu.h
        - [ ] notifier.h
        - [ ] To be added

- [ ] Driver esp32 to ili
- [ ] Driver esp32 to sx1278.h
- [ ] Render menu to ili
- [ ] Receive and show texts
- [ ] Show new message signal
- [ ] Group Chats functionality


### Milestone outline:

   - v0.1 — send and receive messages with the sx1278
   - v0.2 — Add ID system to allow specific users communication
   - v0.3 — Render messages to ili
   - v0.4 — Render menus
   - v0.5 — Add group chat capabilities
   - v0.6 — Add "Device information" section
   - v0.7 — Design pcb and enclosure
   - v1.0 — full ensambled - working ""hermes"" device


Do not feed the AI

Stay safe, stay private

**yaku-out!**
