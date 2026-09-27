# Joystick-controlled ESP32 car

Build a wireless car with two ESP32 boards, an analog joystick, and an L298N motor driver. The boards communicate directly using ESP-NOW; no router or internet connection is needed to drive the car.

The joystick controls speed and direction, including pivot turns and steering while moving. The receiver stops the motors when it has not received a valid command for 500 ms.

## Parts

- Two ESP32 development boards (one transmitter and one receiver)
- Two-axis analog joystick module
- L298N dual motor driver module
- Car chassis, wheels, and DC gear motors
- Motor battery suitable for the motors and driver
- Suitable power supplies for the ESP32 boards
- Jumper wires and USB data cables

Connect the left-side motors to driver channel A and the right-side motors to channel B. Choose motors whose combined current per channel is within the driver's rating.

## Download the project

Clone the repository, or choose **Code > Download ZIP** on GitHub and extract it:

```sh
git clone https://github.com/rohitkr150015/Joystick-Controlled-ESP32-Car.git
cd Joystick-Controlled-ESP32-Car
```

| Folder | Purpose |
| --- | --- |
| `ESP32_MAC_Address/` | Read the receiver board's Wi-Fi MAC address |
| `Transmitter_Joy_Stick_Code/` | Read the joystick and send commands |
| `Receiver_Car_Code/` | Receive commands and control the motors |
| `tests/` | Optional steering calculation tests |

## Wire the transmitter

| Joystick pin | Transmitter ESP32 |
| --- | --- |
| VCC | 3.3V |
| GND | GND |
| VRx | GPIO34 |
| VRy | GPIO35 |
| SW (optional) | GPIO32 |

The joystick button is transmitted but does not control the car. Power the joystick from 3.3V so its analog outputs stay within the ESP32 input range.

![Transmitter wiring](ESPNOW%20RC%20Car%20Transmitter.png)

## Wire the car receiver

| L298N pin | Receiver ESP32 |
| --- | --- |
| ENA | GPIO32 |
| IN1 | GPIO12 |
| IN2 | GPIO14 |
| ENB | GPIO33 |
| IN3 | GPIO27 |
| IN4 | GPIO26 |
| GND | GND |

- Connect the left-side motors to OUT1/OUT2 (channel A).
- Connect the right-side motors to OUT3/OUT4 (channel B).
- Remove the ENA and ENB jumpers so the ESP32 can control motor speed.
- Connect the motor battery to the driver's motor supply terminals. Use the voltage required by your motors and driver module.
- Connect the receiver ESP32 ground, driver ground, and motor battery negative together.
- Power the ESP32 through USB or an appropriate regulated supply. Do not power the motors from the ESP32, or connect the motor battery directly to its 3.3V pin.
- Follow your L298N module's instructions for its separate 5V regulator jumper and logic supply.

![Receiver wiring](ESPNOW%20RC%20Car%20Receiver.png)

## Set up Arduino IDE

1. Install Arduino IDE.
2. In Boards Manager, install **esp32 by Espressif Systems**, version **3.3.11**, to match this project's setup.
3. Select **ESP32 Dev Module** for a standard ESP32 development board, and select the USB port of the board you are programming.

`WiFi.h` and `esp_now.h` are included with the ESP32 board package; no additional Arduino libraries are needed. Keep each `.ino` file in its existing folder, and keep `Steering.h` beside the receiver sketch.

## Configure and upload

### 1. Read the receiver's MAC address

Connect the ESP32 that will be installed on the car. Open and upload:

```text
ESP32_MAC_Address/ESP32_MAC_Address.ino
```

Open Serial Monitor at **115200 baud**, reset the board, and copy the address printed after `ESP32 WiFi MAC:`.

### 2. Program the joystick transmitter

Open:

```text
Transmitter_Joy_Stick_Code/Transmitter_Joy_Stick_Code.ino
```

Replace `receiverMAC` with the address you just copied. For example, an address of `AA:BB:CC:DD:EE:FF` becomes:

```cpp
uint8_t receiverMAC[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
```

Connect the transmitter board, select its USB port, and upload. The address already in the sketch must be replaced with your receiver's address.

### 3. Program the car receiver

Connect the receiver board again. Open and upload:

```text
Receiver_Car_Code/Receiver_Car_Code.ino
```

This replaces the MAC address helper sketch with the car firmware.

## Calibrate the joystick

With both boards powered, release the joystick and read X and Y in Serial Monitor at **115200 baud**. Set these constants in the receiver sketch to your joystick's resting readings:

```cpp
constexpr int JOY_CENTER_X = 1782;
constexpr int JOY_CENTER_Y = 1790;
constexpr int JOY_DEADZONE = 250;
```

The supplied centers are specific to the original joystick. `JOY_DEADZONE` defines the range around each center that counts as neutral. Upload the receiver sketch again after adjusting them.

## Test and drive

Start with the wheels lifted off the ground.

1. Power both boards and the motor driver. The receiver should report `Receiving`.
2. Release the joystick. The receiver should print `Left: 0 | Right: 0` and the motors should stop.
3. Move the joystick forward and backward to check travel direction and speed.
4. Move it sideways to pivot, and diagonally to turn while moving.
5. Turn off the transmitter. The receiver should report `No signal - stopped` and stop the motors after about 500 ms.

Positive motor values mean forward; negative values mean reverse. If one side rotates the wrong way, disconnect power and reverse that side's motor leads. If joystick orientation reverses an axis, negate `throttle` or `steering` in the receiver before the `mixSteering` call, as appropriate.

Once the controls behave as expected, place the car on the ground and begin with small joystick movements.

## Optional: run steering tests

With a C++ compiler such as `g++` installed, run these commands from the repository folder in PowerShell:

```powershell
New-Item -ItemType Directory -Force build | Out-Null
g++ -std=c++11 -Wall -Wextra -pedantic tests/steering_test.cpp -o build/steering_test.exe
./build/steering_test.exe
```

These tests check steering calculations independently of the hardware. Use the driving checks above to verify your assembled car.
