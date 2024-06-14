#include <SDL3/SDL.h>

#include <cmath>
#include <iostream>
#include <numbers>
#include <ostream>
#include <unordered_map>
#include <variant>

#include "common.h"
#include "gui.h"

using std::unordered_map;

auto harmonic = [](float t, float w) -> float { return sinf(2 * std::numbers::pi * t * w); };

auto square = [](float t, float w) -> float {
  if (sinf(2 * M_PI * t * w) >= 0)
    return 1;
  else
    return -1;
};
auto triangle = [](float t, float w) -> float {
  float deg = (360 * t * w);
  deg       = (int)deg % 360;

  if (deg > 0 and deg <= 90) return ((float)deg / 90);
  if (deg > 90 and deg <= 270) return ((float)-deg / 90 + 2);
  if (deg > 270 and deg <= 360)
    return ((float)deg / 90 - 4);
  else
    return 0;
};
auto dc = [](float, float) -> float { return 1; };

class CircuitManager {
 public:
  CircuitManager() {
    input.emplace(InputShape::Harmonic, harmonic);
    input.emplace(InputShape::Square, square);
    input.emplace(InputShape::Triangle, triangle);
    input.emplace(InputShape::DC, dc);
  }

  void update(float offset, float amplitude, InputShape shape, float w) {
    float input_Voltage = amplitude * std::visit([&](auto&& func) { return func(w, currentTime); }, input.at(shape)) + offset;
    state.InputVoltage  = input_Voltage;

    float di = (input_Voltage * deltaT) / params.L - (params.R * state.current * deltaT) / params.L - (params.Ke * state.rot_speed * deltaT / params.L);
    float dw = (params.Kt * state.current * deltaT) / params.I - (params.k * state.rotation * deltaT) / params.I;
    if (std::isnan(dw) || std::isnan(di)) return;
    state.rot_speed += dw;
    state.current += di;

    state.rotation        = state.rotation + state.rot_speed * deltaT * 2 * M_PI;
    state.inductorVoltage = params.L * di / deltaT;
    state.motorVoltage    = params.Ke * state.rot_speed;
    state.ResistorVoltage = state.current * params.R;
    currentTime += deltaT;
  }
  CircutState getState() const { return state; }
  void reset() {
    state       = CircutState{};
    currentTime = 0;
  }
  float getCurrentTime() const { return currentTime; }
  void setParams(CircutParameters params_) { params = params_; };
  void setDeltaT(float deltaT_) { deltaT = deltaT_; }

 private:
  using inputFunctionsMap    = unordered_map<InputShape, std::variant<decltype(harmonic), decltype(square), decltype(triangle), decltype(dc)>>;
  InputShape currentShape = InputShape::Harmonic;
  float      currentTime  = 0;
  float      deltaT       = 0;

  CircutState      state{};
  CircutParameters params{};
  inputFunctionsMap   input{};
};

int main(int, char**) {
  CircuitManager circuit;
  WindowManager  window;
  bool           done = false;
  while (!done) {
    auto options = window.getOptions();
    circuit.setParams(window.getParams());
    circuit.setDeltaT(options.time_step);
    if (options.start_simulation) {
      circuit.update(options.offset, options.amplitude, window.getInputShape(), options.pulse);
      window.addTimeStep(circuit.getCurrentTime());
      window.addState(circuit.getState());
    }
    window.newFrame();
    window.processEvents(done);
    window.renderParametersWindow([&circuit]() { circuit.reset(); });
    window.renderPlotWindow();
    window.render();
  }
}
