#pragma once


struct CircutParameters {
  float R{}; //resistance
  float L{}; //inductance
  float Kt{};
  float Ke{};
  float I{}; //Momentum
  float k{}; //spring
};

struct CircutState {
  float ResistorVoltage{};
  float inductorVoltage{};
  float motorVoltage{};
  float current{};
  float rot_speed{};
  float rotation{};
};
