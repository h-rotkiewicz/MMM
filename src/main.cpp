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

auto harmonic = [](float t, int w) -> float { return sinf(2 * std::numbers::pi * t * w); };

auto square = [](float t, int w) -> float {
  if (sinf(2 * M_PI * t * w) >= 0)
    return 1;
  else
    return -1;
};
auto triangle = [](float t, int w) -> float {
  float deg = (360 * t * w);
  deg       = (int)deg % 360;

  if (deg > 0 and deg <= 90) return ((float)deg / 90);
  if (deg > 90 and deg <= 270) return ((float)-deg / 90 + 2);
  if (deg > 270 and deg <= 360)
    return ((float)deg / 90 - 4);
  else
    return 0;
};

using InputTable = unordered_map<InputShape, std::variant<decltype(harmonic), decltype(square), decltype(triangle)>>;

class CircuitManager {
 public:
  CircuitManager() {
    input.emplace(InputShape::Harmonic, harmonic);
    input.emplace(InputShape::Square, square);
    input.emplace(InputShape::Triangle, triangle);
  }
  void init(CircutParameters params_init) { params = params_init; };

  void update(float amplitude, InputShape shape, uint64_t w, float currentTime, float deltaT) {
    float input_Voltage = amplitude * std::visit([=](auto&& func) { return func(w, currentTime); }, input.at(shape));
    state.InputVoltage  = input_Voltage;

    float di = (input_Voltage * deltaT) / params.L - (params.R * state.current * deltaT) / params.L - (params.Ke * state.rot_speed * deltaT / params.L);
    float dw = (params.Kt * state.current * deltaT) / params.I - (params.k * state.rotation * deltaT) / params.I;

    state.rot_speed += dw;
    state.current += di;

    state.rotation        = state.rotation + state.rot_speed * deltaT;
    state.inductorVoltage = params.L * di / deltaT;
    state.motorVoltage    = params.Ke * dw / deltaT;
    state.ResistorVoltage = state.current * params.R;

#ifdef DEBUG
    std::cout << "Input Voltage: " << input_Voltage << std::endl;
    std::cout << "Circuit Current: " << state.current << std::endl;
    std::cout << "Rotational Speed: " << state.rot_speed << std::endl;

#endif
  }

  auto get_state() { return state; }
  auto get_input(uint64_t w) {
    return std::visit([=, this](auto&& func) { return func(w, currentTime); }, input.at(currentShape));
  }

  void set_input_shape(InputShape shape) { currentShape = shape; }

 private:
  InputShape currentShape = InputShape::Harmonic;
  float      currentTime  = 1;
  float      delatTime    = 0.01;

  CircutState      state{};
  CircutParameters params{};
  InputTable       input{};
};

int main(int, char**) {
  CircuitManager circut;
  WindowManager window;
  float         time_step    = 0.01;
  float         current_time = 0;
  bool          done         = false;
  while (!done) {
    auto options = window.getOptions();
    circut.init(window.getParams());
    if (options.start_simulation) {
      circut.update(options.amplitude, window.getInputShape(), options.width, current_time, time_step);
      window.add_timeStep(current_time);
      window.add_state(circut.get_state());
      current_time += time_step;
    }
    window.newFrame();
    window.process_events(done);
    window.render_parameters_window();
    window.render_plot_window();
    window.render();
  }
}
