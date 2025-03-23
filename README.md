# Simple Wii nunchuk driver on linux

## Overview
Linux driver for the Nintendo Wii Nunchuk. Creates a virtual linux input event device, creating a polling thread whenever opened and reports data approximately every 16 ms. 

## Features

- Alpha release
- Compatible with **Linux kernel version [6.6]**.
- Wii nunchuk device tree overlay.
- Virtual linux input event gamepad with joystick, two trigger buttons, 3 DOF accelerator inputs.
- Works on the raspberry pi 4B

## Installation

### Prerequisites

Install the following dependencies:

```sh
sudo apt install -y build-essential make xz
```
If using the raspberry pi os, install the raspberry pi kernel headers:

```sh
sudo apt install raspberrypi-kernel-headers
```
### Build & Install

Clone the repository:

```sh
git clone https://github.com/Gotnam/wiinunchuklinux.git
cd wiinunchuklinux
```

Edit the KDIR variable in the Makefile to point to the kernel header build directory. If you are using the raspberry pi os or are otherwise building on the host machine, then the directory is:

```make
KDIR := /lib/modules/$(uname -r)/build
```

Then run make:

```sh
make all
sudo make install
```

