# PlatypusScript
HID scripting language for RaspberryPi with GPIO and Eth support

***If it looks like a duck, swims like a duck and quacks lke a duck, then it probably is a duck. If it has a bunch of other shit attached, then it's a Platypus.***


![Hacker Platypus can't hurt you](images/platypus.png)

Inspired on the (in)famous DuckyScript but I wanted to do stuff over Ethernet and GPI too so I wrote an interpretter from scratch to accommodate the new functionality.

## Setup
To clone this repository with its dependencies, use:
```bash
git clone --recurse-submodules https://github.com/lycheegirl/PlatypusScript
```

## Usage
### For Pi Pico 2W (might work on others, haven't tried)
```bash
cd /path/to/where/you/cloned/the/repo/PlatypusScript/platform_pico
cmake -DPICO_SDK_PATH=lib/pico-sdk
make -j$(nproc)
```

Then flash the Pi Pico with `platypus.uf2`.

#### For the payload:
I used a 4 pin micro SD card board module with jumper wires but as long as you wire it up correctly it doesn't matter
- GND to GND
- VCC to Pi 5V
- MISO to GPIO 16 (Physical pin 21)
- MOSI to GPIO 19 (Physical pin 26)
- SCK to GPIO 18 (Physical pin 24)
- CS to GPIO 17 (Physical pin 22)

[Read hte official pinout chart here](https://pip-assets.raspberrypi.com/categories/1088-raspberry-pi-pico-2-w/documents/RP-008305-DS-1-pico-2-w-pinout.pdf)

Next steps:
1. Format an Micro SD card with FAT32
2. Create a new file called payload.txt
3. Write your payload script and save it to the root of the card
4. Eject and plug using USB- Micro USB adapter
5. Watch as your Pi gets enumerated as a keyboard and types for you

### For Pi Zero
Coming soon maybe
