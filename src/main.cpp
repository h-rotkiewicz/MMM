#include <SDL3/SDL.h>
#include <stdio.h>

#include <cmath>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <unordered_map>
#include <variant>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif
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

struct CircutParameters {
  uint64_t Resistance{};
  uint64_t inductance{};
  uint64_t Kt{};
  uint64_t Ke{};
};

struct CircutState {
  int64_t ResistorVoltage{};
  int64_t inductorVoltage{};
  int64_t motorVoltage{};
  int64_t current{};
};

class CircutManager {
 public:
  CircutManager() {
    input.emplace(InputShape::Harmonic, harmonic);
    input.emplace(InputShape::Square, square);
    input.emplace(InputShape::Triangle, triangle);
  }

  UpdateType update(uint64_t w) {
    float Voltage =
	std::visit([&, this](auto&& func) { return func(w, currentTime); }, input.at(currentShape));
#ifdef Debug
    std::cout << "Input Voltage: " << Voltage << std::endl;
#endif
    // TODO: implement the update function
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
  return {CircutState{0, 0, 0, 0},
	  CircutParameters{.Resistance = 1, .inductance = 1, .Kt = 1, .Ke = 2}};
}

int main(int, char**) {
  constexpr static auto background_color = ImVec4(0.55f, 0.55f, 0.60f, 1.00f);
  Window window;
  bool	 done = false;
  while (!done) {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    window.process_events(done);
    window.render_imgui_window();
    window.render();
  }
}
