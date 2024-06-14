#pragma once

struct CircutParameters {
  float R  = 1;
  float L  = 1;
  float Kt = 1;
  float Ke = 1;
  float I  = 1;
  float k  = 1;
};

struct CircutState {
  float ResistorVoltage{};
  float inductorVoltage{};
  float motorVoltage{};
  float current{};
  float rot_speed{};
  float rotation{};
  float InputVoltage{};
};

enum class InputShape {
  Harmonic,
  Square,
  Triangle,
  DC,
};
