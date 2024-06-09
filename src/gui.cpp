#include "gui.h"

#include <implot.h>

#include <cmath>
#include <stdexcept>

#include "SDL3/SDL_hints.h"
#include "SDL3/SDL_init.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

std::tuple<SDL_Window*, SDL_Renderer*> Window::init_backend() {
  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMEPAD) != 0) throw std::runtime_error("SDL_Init failed");

  // Enable native IME.
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

  // Create window with SDL_Renderer graphics context
  Uint32      window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
  SDL_Window* window       = SDL_CreateWindow("Dear ImGui SDL3+SDL_Renderer example", 1280, 720, window_flags);
  if (window == nullptr) throw std::runtime_error("SDL_CreateWindow failed");
  SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
  SDL_SetRenderVSync(renderer, 1);
  if (renderer == nullptr) throw std::runtime_error("SDL_CreateRenderer failed");
  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(window);

  return {window, renderer};
}

auto Window::init_imgui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

  ImGui::StyleColorsDark();
  return io;
}

Window::Window() {
  auto [window_, renderer_] = init_backend();
  window                    = window_;
  renderer                  = renderer_;
  ImGuiIO io                = init_imgui();
  ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer3_Init(renderer);
  ImPlot::CreateContext();
}

void Window::newFrame(){
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
}

void Window::render_imgui_window() {
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(300, ImGui::GetIO().DisplaySize.y));  // Adjust the width (300) as needed

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
  ImGui::Begin("Left Side Window", nullptr, window_flags);

  const char* inputTypeItems[] = {"Harmonic", "Square", "Triangle"};
  ImGui::Combo("Input Type", &inputType, inputTypeItems, IM_ARRAYSIZE(inputTypeItems));
  ImGui::InputFloat("Resistance", &params.R);
  ImGui::InputFloat("Inductance", &params.I);
  ImGui::InputFloat("Kt", &params.Kt);
  ImGui::InputFloat("Ke", &params.Ke);
  ImGui::InputFloat("time step", &time_step);
  ImGui::Checkbox("Start Simulation", &start_simulation);
  ImGui::End();
}

void Window::render_plot_window() {
    // Populate test data (this should be removed in the actual implementation)
    for (float i = 0; i < 100; ++i) {
        states.push_back(CircutState{ 0, 0, 0, static_cast<float>(sin(0.2 * i)), 0});
        timeSteps.push_back(static_cast<std::time_t>(i));
    }

    ImGui::SetNextWindowPos(ImVec2(300, 0));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - 300, ImGui::GetIO().DisplaySize.y / 2));  
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Graph Window", nullptr, window_flags);

    if (ImPlot::BeginPlot("Circuit Data")) {
        std::vector<float> times;
        std::vector<float> values;

        for (size_t i = 0; i < states.size(); ++i) {
            times.push_back(static_cast<float>(timeSteps[i]));
            values.push_back(states[i].current);  
        }

        ImPlot::PlotLine("current Value Over Time", times.data(), values.data(), static_cast<int>(times.size()));

        ImPlot::EndPlot();
    }

    ImGui::End();
}

void Window::process_events(bool& done) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);
    if (event.type == SDL_EVENT_QUIT) done = true;
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.keysym.sym == SDLK_ESCAPE) done = true;
    if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window)) done = true;
  }
}

void Window::render() {
  ImGui::Render();
  SDL_SetRenderDrawColor(renderer, (Uint8)(background_color.x * 255), (Uint8)(background_color.y * 255), (Uint8)(background_color.z * 255), (Uint8)(background_color.w * 255));
  SDL_RenderClear(renderer);
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
  SDL_RenderPresent(renderer);
}
