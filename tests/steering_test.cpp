#include <cassert>
#include <iostream>
#include "../Receiver_Car_Code/Steering.h"

int main() {
  const int centerX = 1782, centerY = 1790, deadzone = 250;
  for (int value = centerX - deadzone; value <= centerX + deadzone; ++value) {
    assert(axisSpeed(value, centerX, deadzone) == 0);
  }
  assert(axisSpeed(0, centerX, deadzone) == -255);
  assert(axisSpeed(4095, centerX, deadzone) == 255);
  // Original left-turn mapping returned a negative duty at X=1800.
  assert(axisSpeed(1800, centerX, deadzone) == 0);
  assert(axisSpeed(1400, centerX, deadzone) < 0);
  for (int center : {centerX, centerY}) {
    int previous = -255;
    for (int value = -100; value <= 4200; ++value) {
      int speed = axisSpeed(value, center, deadzone);
      assert(speed >= -255 && speed <= 255 && speed >= previous);
      previous = speed;
    }
  }
  MotorSpeeds stopped = mixSteering(0, 0);
  assert(stopped.left == 0 && stopped.right == 0);
  MotorSpeeds right = mixSteering(0, 255);
  assert(right.left == 255 && right.right == -255);
  MotorSpeeds left = mixSteering(0, -255);
  assert(left.left == -255 && left.right == 255);
  for (int throttle = -255; throttle <= 255; ++throttle) {
    MotorSpeeds straight = mixSteering(throttle, 0);
    assert(straight.left == throttle && straight.right == throttle);
    for (int steering = -255; steering <= 255; ++steering) {
      MotorSpeeds speeds = mixSteering(throttle, steering);
      assert(speeds.left >= -255 && speeds.left <= 255);
      assert(speeds.right >= -255 && speeds.right <= 255);
      if (steering > 0) assert(speeds.left > speeds.right);
      if (steering < 0) assert(speeds.left < speeds.right);
    }
  }
  // Logged Y=1791 bypassed steering entirely, even at full X deflection.
  MotorSpeeds noisyRight = mixSteering(axisSpeed(1791, centerY, deadzone),
                                       axisSpeed(4095, centerX, deadzone));
  assert(noisyRight.left == 255 && noisyRight.right == -255);
  MotorSpeeds loggedLeft = mixSteering(axisSpeed(1790, centerY, deadzone),
                                      axisSpeed(0, centerX, deadzone));
  assert(loggedLeft.left == -255 && loggedLeft.right == 255);
  std::cout << "Steering tests passed\n";
}
