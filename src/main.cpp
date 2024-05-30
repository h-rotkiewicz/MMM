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

  UpdateType update(uint64_t w) {
    float Voltage = std::visit([&,this](auto&& func) { return func(w, currentTime); }, input.at(currentShape));
#ifdef Debug
    std::cout << "Input Voltage: " << Voltage << std::endl;
#endif
    // TODO: implement the update function
    return UpdateType::SteadyState;
  }

  auto get_input(uint64_t w){
    return std::visit([=,this](auto&& func) { return func(w, currentTime); }, input.at(currentShape)); 
  }

  void set_input_shape(InputShape shape) { currentShape = shape; }

 private:
  InputShape	   currentShape = InputShape::Harmonic;
  float currentTime = 1;
  float delatTime = 0.01;;
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

void process_events(bool& done, auto const& window) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);
    if (event.type == SDL_EVENT_QUIT) done = true;
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.keysym.sym == SDLK_ESCAPE) done = true;
    if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window)) done = true;
  }
}

void render_imgui_window() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(300, ImGui::GetIO().DisplaySize.y)); // Adjust the width (300) as needed

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Left Side Window", nullptr, window_flags);

    static int inputType = 0;
    static float resistance = 0.0f;
    static float inductance = 0.0f;
    static float Kt = 0.0f;
    static float Ke = 0.0f;

    const char* inputTypeItems[] = { "Harmonic", "Square", "Triangle" };
    ImGui::Combo("Input Type", &inputType, inputTypeItems, IM_ARRAYSIZE(inputTypeItems));
    ImGui::InputFloat("Resistance", &resistance);
    ImGui::InputFloat("Inductance", &inductance);
    ImGui::InputFloat("Kt", &Kt);
    ImGui::InputFloat("Ke", &Ke);
    ImGui::End();
}

void render_canvas(SDL_Renderer* renderer) {
  // Set the destination rectangle for the canvas to occupy the right side of the screen
  const auto window_height  = ImGui::GetIO().DisplaySize.y;
  const auto window_width   = ImGui::GetIO().DisplaySize.x;
  auto*	     canvas_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, window_width - 300, window_height);
  SDL_FRect  canvas_rect    = {300, 0, (window_width - 300), (window_height)};

  SDL_SetRenderTarget(renderer, canvas_texture);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_FRect example_rect = {50, 50, 200, 150};

  SDL_RenderFillRect(renderer, &example_rect); //TODO: Draw the circut here

  SDL_SetRenderTarget(renderer, nullptr);

  SDL_RenderTexture(renderer, canvas_texture, nullptr, &canvas_rect);
}

void preRender() {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
}

int main(int, char**) {
  constexpr auto background_color = ImVec4(0.55f, 0.55f, 0.60f, 1.00f);
  // Our state
  auto [window, renderer] = init_backend();
  auto io		  = init_imgui();
  ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer3_Init(renderer);
  bool done = false;
  while (!done) {
    preRender();
    process_events(done, window);
    render_imgui_window();
    render_canvas(renderer);
    ImGui::Render();
    SDL_SetRenderDrawColor(renderer, (Uint8)(background_color.x * 255), (Uint8)(background_color.y * 255), (Uint8)(background_color.z * 255), (Uint8)(background_color.w * 255));
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
  }
}
