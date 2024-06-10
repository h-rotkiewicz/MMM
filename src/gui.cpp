#include "gui.h"

#include <implot.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <ostream>
#include <stdexcept>

#include "SDL3/SDL_hints.h"
#include "SDL3/SDL_init.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

std::tuple<SDL_Window*, SDL_Renderer*> WindowManager::init_backend() {
  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMEPAD) != 0) throw std::runtime_error("SDL_Init failed");

  // Enable native IME.
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

  // Create window with SDL_Renderer graphics context
  Uint32      window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
  SDL_Window* window       = SDL_CreateWindow("Circut simulation", 1280, 720, window_flags);
  if (window == nullptr) throw std::runtime_error("SDL_CreateWindow failed");
  SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
  SDL_SetRenderVSync(renderer, 1);
  if (renderer == nullptr) throw std::runtime_error("SDL_CreateRenderer failed");
  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(window);

  return {window, renderer};
}

auto WindowManager::init_imgui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

  ImGui::StyleColorsDark();
  return io;
}

WindowManager::WindowManager() {
  auto [window_, renderer_] = init_backend();
  window                    = window_;
  renderer                  = renderer_;
  ImGuiIO io                = init_imgui();
  ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer3_Init(renderer);
  ImPlot::CreateContext();
}

void WindowManager::newFrame() {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
}

void WindowManager::render_parameters_window() {
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(300, ImGui::GetIO().DisplaySize.y));  // Adjust the width (300) as needed

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
  ImGui::Begin("Left Side Window", nullptr, window_flags);

  const char* inputTypeItems[] = {"Harmonic", "Square", "Triangle", "dc"};
  ImGui::Combo("Input Type", &options.inputType, inputTypeItems, IM_ARRAYSIZE(inputTypeItems));
  ImGui::InputFloat("dc offset", &options.offset);
  ImGui::InputFloat("omega", &options.width);
  ImGui::InputFloat("Resistance", &params.R);
  ImGui::InputFloat("Inductance", &params.L);
  ImGui::InputFloat("Kt", &params.Kt);
  ImGui::InputFloat("Ke", &params.Ke);
  ImGui::InputFloat("time step", &options.time_step, 0.0, 0.0f, "%.10f");
  ImGui::InputFloat("Momentum", &params.I);
  ImGui::InputFloat("Stiffness", &params.k);
  ImGui::InputFloat("width (w)", &options.width);
  ImGui::InputFloat("amplitude", &options.amplitude);
  ImGui::Checkbox("Start Simulation", &options.start_simulation);
  ImGui::Checkbox("auto scroll", &options.auto_scroll);
  ImGui::BulletText("Press 'ESC' to exit the program");
  ImGui::BulletText("Currently Saved States: %zu", states.size());
  ImGui::BulletText("Currently Saved Time steps: %zu", timeSteps.size());
  ImGui::Text("Current Values: ");
  if (states.size() > 0) {
    ImGui::BulletText("current %f", states.back().current);
    ImGui::BulletText("rot_speed %f", states.back().rot_speed);
    ImGui::BulletText("rotation %f", states.back().rotation);
    ImGui::BulletText("Input Voltage %f", states.back().InputVoltage);
    ImGui::BulletText("Resistor Voltage %f", states.back().ResistorVoltage);
    ImGui::BulletText("Inductor Voltage %f", states.back().inductorVoltage);
    ImGui::BulletText("Motor Voltage %f", states.back().motorVoltage);
    ImGui::BulletText("Time Step %f", timeSteps.back());
  }

  if (ImGui::Button("Clear Data")) {
    states.clear();
    timeSteps.clear();
  }
  ImGui::End();
}

void WindowManager::render_plot_window() {
  ImGui::SetNextWindowPos(ImVec2(300, 0));
  ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - 300, ImGui::GetIO().DisplaySize.y));
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
  ImGui::Begin("Graph Window", nullptr, window_flags);

  if (ImPlot::BeginPlot("Circuit Data", ImVec2(-1, ImGui::GetIO().DisplaySize.y-35))) { // -35 to account for the ImGui stuff
    if (timeSteps.size() > 0 && options.auto_scroll) ImPlot::SetupAxisLimits(ImAxis_X1, timeSteps.back() - 10, timeSteps.back(), ImGuiCond_Always);
    auto plotLine = [&](const char* label, auto valueGetter) {
      std::vector<float> values(states.size());
      std::transform(states.begin(), states.end(), values.begin(), valueGetter);
      ImPlot::PlotLine(label, timeSteps.data(), values.data(), timeSteps.size());
    };
    if (timeSteps.size() > 0) {
      plotLine("ResistorVoltage", [](const CircutState& s) { return s.ResistorVoltage; });
      plotLine("inductorVoltage", [](const CircutState& s) { return s.inductorVoltage; });
      plotLine("motorVoltage", [](const CircutState& s) { return s.motorVoltage; });
      plotLine("current", [](const CircutState& s) { return s.current; });
      plotLine("rotation speed", [](const CircutState& s) { return s.rot_speed; });
      plotLine("rotations", [](const CircutState& s) { return s.rotation; });
      plotLine("Input voltage", [](const CircutState& s) { return s.InputVoltage; });
    }
    ImPlot::EndPlot();
  }
  ImGui::End();
}

void WindowManager::process_events(bool& done) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);
    if (event.type == SDL_EVENT_QUIT) done = true;
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.keysym.sym == SDLK_ESCAPE) done = true;
    if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window)) done = true;
  }
}

void WindowManager::render() {
  ImGui::Render();
  SDL_SetRenderDrawColor(renderer, (Uint8)(background_color.x * 255), (Uint8)(background_color.y * 255), (Uint8)(background_color.z * 255), (Uint8)(background_color.w * 255));
  SDL_RenderClear(renderer);
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
  SDL_RenderPresent(renderer);
}


//Getters and setters
void WindowManager::add_timeStep(float timeStep) { timeSteps.push_back(timeStep); }
void WindowManager::add_state(CircutState state) { states.push_back(state); }

CircutParameters WindowManager::getParams() const { return params; }
WindowOptions WindowManager::getOptions() const { return options; }

InputShape WindowManager::getInputShape() const {
  if (options.inputType == 0)
    return InputShape::Harmonic;
  else if (options.inputType == 1)
    return InputShape::Square;
  else if (options.inputType == 2)
    return InputShape::Triangle;
  else if (options.inputType == 3)
    return InputShape::DC;
  throw std::runtime_error("Invalid Input Shape");
}
