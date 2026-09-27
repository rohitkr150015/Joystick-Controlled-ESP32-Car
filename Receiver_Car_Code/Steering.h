#pragma once

struct MotorSpeeds {
  int left;
  int right;
};

// Convert a 12-bit joystick reading into a signed PWM command.
inline int axisSpeed(int value, int center, int deadzone) {
  if (value < 0) value = 0;
  if (value > 4095) value = 4095;
  if (value > center + deadzone) {
    return (value - center - deadzone) * 255 / (4095 - center - deadzone);
  }
  if (value < center - deadzone) {
    return -(center - deadzone - value) * 255 / (center - deadzone);
  }
  return 0;
}

inline MotorSpeeds mixSteering(int throttle, int steering) {
  // At zero throttle this pivots; with throttle it makes a curved turn.
  int left = throttle + steering;
  int right = throttle - steering;
  int absLeft = left < 0 ? -left : left;
  int absRight = right < 0 ? -right : right;
  int peak = absLeft > absRight ? absLeft : absRight;
  if (peak > 255) {
    left = left * 255 / peak;
    right = right * 255 / peak;
  }
  return {left, right};
}
