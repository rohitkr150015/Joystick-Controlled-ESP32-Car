# Joystick-controlled ESP32 car

An ESP-NOW remote-controlled car using two ESP32 boards, an analog joystick,
and an L298N motor driver. The receiver supports proportional driving,
pivot turns, steering while moving, and stopping on signal loss.

## Project files

- `Transmitter_Joy_Stick_Code/`: joystick transmitter (X=GPIO34, Y=GPIO35).
- `Receiver_Car_Code/`: car receiver and steering calculations.
- `ESP32_MAC_Address/`: helper sketch for reading the receiver's MAC address.
- `tests/`: host C++ steering tests.

For a new pair of boards, run the MAC address helper on the receiver and
copy its address into `receiverMAC` in the transmitter sketch before
uploading the transmitter. The checked-in address belongs to the original
setup. The joystick button is optional and does not control the motors.

## Wiring diagrams

![Receiver wiring](ESPNOW%20RC%20Car%20Receiver.png)

![Transmitter wiring](ESPNOW%20RC%20Car%20Transmitter.png)

## Steering fix

The original receiver tested Y before X in an `if / else if` chain. Captured logs
shows a resting Y near 1790, below its backward threshold of 1825. Even
`X=4095, Y=1790` and `X=0, Y=1791` therefore selected backward at PWM 4-5
instead of turning. Its left-turn map also produced negative PWM values for
X between 1401 and 1824.

The receiver now uses centers X=1782 and Y=1790 inferred from those logs, with
a deadzone of 250 counts on each side. It combines throttle and steering:
left motor = throttle + steering; right motor = throttle - steering.
Motor commands are scaled into -255..255, with direction handled separately
from PWM duty. Sideways joystick movement pivots; diagonal movement turns
while driving. It stops before the first packet and after 500 ms without a
valid packet. Debug output is limited to four lines per second.

## Upload and check

1. Open `Receiver_Car_Code/Receiver_Car_Code.ino` in Arduino IDE. Keep
   `Steering.h` in that same folder. Select ESP32 Dev Module and the car's
   USB port, then upload. The existing transmitter sketch is compatible.
2. Start with the wheels lifted. Open Serial Monitor at 115200 baud.
3. Release the joystick: `Left: 0 | Right: 0 | Receiving` is expected.
   If needed, adjust `JOY_CENTER_X` and `JOY_CENTER_Y` to the released values.
4. With Y near 1790, X=4095 should print `Left: 255 | Right: -255`;
   X=0 should print `Left: -255 | Right: 255`. Positive means forward and
   negative means reverse. Intermediate deflections produce lower speed.
5. Check forward/backward and diagonals. Switch off the transmitter: the
   receiver should print `No signal - stopped` and stop within about 500 ms.

Motor A (ENA=32, IN1=12, IN2=14) is assumed to drive the left side; motor B
(ENB=33, IN3=27, IN4=26) the right side. These pin assignments and forward
polarities are retained from the original sketch. If forward/backward work
but left/right are reversed, negate `steering` before calling `mixSteering`.
If the printed motor commands are correct but a motor does not respond,
check that motor's driver connections and power.

## Validation

The receiver compiled successfully for `esp32:esp32:esp32` (ESP32 Dev Module)
using Arduino-ESP32 3.3.11. Host steering tests passed. Physical operation
of the revised firmware still needs to be verified on the car.

Run the host steering tests from the project folder:

```powershell
New-Item -ItemType Directory -Force build | Out-Null
g++ -std=c++11 -Wall -Wextra -pedantic tests/steering_test.cpp -o build/steering_test.exe
./build/steering_test.exe
```

The tests cover center deadzones, full travel, the logged left/right failure
cases, monotonic speed mapping, and all 261,121 combinations of signed
throttle/steering commands. They do not verify physical motor operation.

The callback only copies validated data; logging and motor control run in
`loop()`, consistent with [Espressif's advice to keep ESP-NOW callbacks short](https://docs.espressif.com/projects/esp-idf/en/v5.2/esp32/api-reference/network/esp_now.html).
