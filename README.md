# PlatypusScript
HID scripting language for RaspberryPi with GPIO and Eth support

***If it looks like a duck, swims like a duck and quacks lke a duck, then it probably is a duck. If it has a bunch of other shit attached, then it's a Platypus.***


![Hacker Platypus can't hurt you](images/platypus.png)
<sub>Image my Gemini because I can't draw and I'm too cheap to pay someone for a throwaway image</sub>

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

[Read the official pinout chart here](https://pip-assets.raspberrypi.com/categories/1088-raspberry-pi-pico-2-w/documents/RP-008305-DS-1-pico-2-w-pinout.pdf)

### Next steps:
1. Format an Micro SD card with FAT32
2. Create a new file called payload.txt
3. Write your payload script and save it to the root of the card
4. Eject and plug using USB- Micro USB adapter
5. Watch as your Pi gets enumerated as a keyboard and types for you

## PlatypusScript Supported Commands
### Env
- `LANG` - sets active keyboard language layout context. Only evaluated if it appears on line 1.
- `SET <var> <val>` - initializes or updates a variable. accepts an integer literal or the value of another variable. Doesn't yet support Math, coming soon<sup>TM</sup>
- `REM` - denotes a comment

### Conditional blocks
- `IF <condition>` - evaluates a condition and begind a conditional execution block
- `ELSE IF <condition>` - evaluates an alternate state if the preceding `IF` evaluates to false
- `ELSE` - executes a fallback block of code if no previoud `IF` or `ELSEIF` was truthy
- `ENDIF` - closes out a conditional block

### Loops
- `DO` — establishes the starting marker position for a conditional loop block
- `WHILE <condition>` — evaluates a condition and loops back to the nearest preceding `DO` statement if true
- ______
- `FOR <variable> <start> TO <end>` — initializes an integer tracking counter variable and sets a looping anchor block
- `ENDFOR` — increments the designated `FOR` counter by 1 and loops back to the anchor if it has not yet exceeded the target end value

### Execution/Output
- `STRING <text>` - parses and transmits text charactesr as UTF-8 codepoints
- `SLEEP <ms>` - suspends script execution for x number os milliseconds

### Example Script
```
LANG US
REM PlatypusScript demo

SET target_val 50
SET loop_active 1

REM 1. Testing Conditional Logic Blocks
IF target_val == 10
    STRING Value is ten.
ELSE IF target_val > 20
    STRING Value is greater than twenty.
ELSE
    STRING Value is small.
ENDIF

SLEEP 200

REM 2. Testing the FOR Loop
FOR i 1 TO 3
    STRING Iteration step...
ENDFOR

REM 3. Testing the DO / WHILE Loop
REM (break it manually by changing the variable inside, math support coming soon)
DO
    STRING Inside the DO loop exactly once!
    SET loop_active 0
WHILE loop_active == 1

STRING Script complete!
```

## Coming soon (in no particular order):
- Math support for `SET` variables
- PlatypusScript docs for payload writing
- Pi Zero compatbility
- ETH packet control
- Waveshare Pico LCD 1.14" screen for payload selection and other interactive stuff

