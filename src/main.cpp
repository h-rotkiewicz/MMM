#include <SDL3/SDL.h>
#include <stdio.h>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <unordered_map>
#include <variant>

#include "common.h"
#include "gui.h"

using std::unordered_map;

enum class UpdateType {
  SteadyState,
  Transient
};
enum class InputShape {
  Harmonic,
  Square,
  Triangle
};

auto harmonic = [](time_t t, uint64_t w) -> float { return sinf(2 * M_PI * t * w); };


auto square = [](uint64_t t, uint64_t w) -> float {
  if (sinf(2 * M_PI * t * w) >= 0)
    return 1;
  else
    return -1;
};
auto triangle = [](float t, uint64_t w) -> float {
  float deg = (360 * t * w);
  deg	    = (int)deg % 360;

  if (deg > 0 and deg <= 90) return ((float)deg / 90);
  if (deg > 90 and deg <= 270) return ((float)-deg / 90 + 2);
  if (deg > 270 and deg <= 360)
    return ((float)deg / 90 - 4);
  else
    return 0;
};

using InputTable =
    unordered_map<InputShape,
		  std::variant<decltype(harmonic), decltype(square), decltype(triangle)>>;

class CircutManager {
 public:
  CircutManager() {
    input.emplace(InputShape::Harmonic, harmonic);
    input.emplace(InputShape::Square, square);
    input.emplace(InputShape::Triangle, triangle);
  }
  void init(CircutState state_init, CircutParameters params_init)
  {
    state = state_init;
    params = params_init;
  };

  UpdateType update(InputShape shape, uint64_t w ,float currentTime,float deltaT) {
    float input_Voltage = 10; // TODO: implement inputshape(currentTime)
    //std::visit([=](auto&& func) { return func(w, currentTime); }, input.at(shape));

    float di = (input_Voltage*deltaT)/params.L-(params.R*state.current*deltaT)/params.L-(params.Ke*state.rot_speed*deltaT/params.L);
    float dw = (params.Kt*state.current*deltaT)/params.I - (params.k*state.rotation*deltaT)/params.I;

    state.rot_speed += dw;
    state.current += di;

    state.rotation = state.rotation + state.rot_speed*deltaT;
    state.inductorVoltage = params.L*di/deltaT;
    state.motorVoltage = params.Ke*dw/deltaT;
    state.ResistorVoltage = state.current*params.R;


#ifdef DEBUG 
    std::cout << "Input Voltage: " << input_Voltage <<std::endl;
    std::cout << "Circuit Current: " << state.current <<std::endl;
    std::cout << "Rotational Speed: " << state.rot_speed <<std::endl;

#endif
   
    return UpdateType::SteadyState;
  }

  auto get_input(uint64_t w) {
    return std::visit([=, this](auto&& func) { return func(w, currentTime); },
		      input.at(currentShape));
  }

  void set_input_shape(InputShape shape) { currentShape = shape; }

 private:
  InputShape currentShape = InputShape::Harmonic;
  float	     currentTime  = 1;
  float	     delatTime	  = 0.01;
  ;
  CircutState	   state{};
  CircutParameters params{};
  InputTable	   input{};
};

std::pair<CircutState, CircutParameters> getInitalState() {
  /* The function will probably serve as a callback for the TGUI library,
   * meaning it will collect data entered by a user in a gui and init
   * CircutState and CircutParameters for now it returns sample conditions for
   * ease of development*/

  return {CircutState{0, 0, 0, 0, 0},
	  CircutParameters{1, 1, 1, 1, 1, 1}};
}

int main(int, char**) {
  CircutManager circut;
  Window window;
  float time_step = 0.01;
  float current_time = 0;
  bool	 done = false;
  while (!done) {
    circut.init(getInitalState().first, getInitalState().second);
    circut.update(InputShape::Harmonic, 1, current_time, time_step);
    window.newFrame();
    window.process_events(done);
    window.render_imgui_window();
    window.render_plot_window();
    window.render();
    current_time += time_step;
  }
}
