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

using InputTable = unordered_map<InputShape, std::variant<decltype(harmonic), decltype(square), decltype(triangle)>>;

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

  UpdateType update(InputShape shape, uint64_t w, float currentTime, float deltaT) {
    float Voltage = std::visit([=](auto&& func) { return func(w, currentTime); }, input.at(shape));
#ifdef Debug
    std::cout << "Input Voltage: " << Voltage << std::endl;
#endif
    // TODO: implement the update function
    return UpdateType::SteadyState;
  }

 private:
  CircutState	   state{};
  CircutParameters params{};
  InputTable	   input{};
};

std::pair<CircutState, CircutParameters> getInitalState() {
  /* The function will probably serve as a callback for the TGUI library,
   * meaning it will collect data entered by a user in a gui and init
   * CircutState and CircutParameters for now it returns sample conditions for
   * ease of development*/
  return {CircutState{0, 0, 0, 0}, CircutParameters{.Resistance = 1, .inductance = 1, .Kt = 1, .Ke = 2}};
}

std::tuple<SDL_Window*, SDL_Renderer*> init_backend() {
  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMEPAD) != 0) throw std::runtime_error("SDL_Init failed");

  // Enable native IME.
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

  // Create window with SDL_Renderer graphics context
  Uint32      window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
  SDL_Window* window	   = SDL_CreateWindow("Dear ImGui SDL3+SDL_Renderer example", 1280, 720, window_flags);
  if (window == nullptr) throw std::runtime_error("SDL_CreateWindow failed");
  SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
  SDL_SetRenderVSync(renderer, 1);
  if (renderer == nullptr) throw std::runtime_error("SDL_CreateRenderer failed");
  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(window);

  return {window, renderer};
}

auto init_imgui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	 // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	 // Enable Gamepad Controls

  ImGui::StyleColorsDark();
  return io;
}

int main(int, char**) {
  // Our state
  auto [window, renderer] = init_backend();
  auto io		  = init_imgui();
  ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer3_Init(renderer);

  bool done = false;
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT) done = true;
      if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window)) done = true;
    }

  }
}
