#include "gui.h"

#include <implot.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <stdexcept>

#include "SDL3/SDL_hints.h"
#include "SDL3/SDL_init.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

std::tuple<SDL_Window*, SDL_Renderer*> WindowManager::initBackend() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMEPAD) != 0) throw std::runtime_error("SDL_Init failed");

  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

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

ImGuiIO WindowManager::initImgui() {
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
  auto [window_, renderer_] = initBackend();
  window                    = window_;
  renderer                  = renderer_;
  ImGuiIO io                = initImgui();
  ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer3_Init(renderer);
  ImPlot::CreateContext();
}

void WindowManager::newFrame() {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
}

void WindowManager::renderPlotWindow() {
  ImGui::SetNextWindowPos(ImVec2(300, 0));
  ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - 300, ImGui::GetIO().DisplaySize.y));
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
  ImGui::Begin("Graph Window", nullptr, window_flags);

  if (ImPlot::BeginPlot("Circuit Data", ImVec2(-1, ImGui::GetIO().DisplaySize.y - 35))) {  // -35 to account for the ImGui stuff
    if (timeSteps.size() > 0 && options.auto_scroll) ImPlot::SetupAxisLimits(ImAxis_X1, timeSteps.back() - 10, timeSteps.back(), ImGuiCond_Always);

    if (timeSteps.size() > 0) {
      ImPlot::PlotLine("Resistor voltage", timeSteps.data(), States.resistorVoltage.data(), timeSteps.size());
      ImPlot::PlotLine("Inductor voltage", timeSteps.data(), States.inductorVoltage.data(), timeSteps.size());
      ImPlot::PlotLine("Motor voltage", timeSteps.data(), States.motorVoltage.data(), timeSteps.size());
      ImPlot::PlotLine("Current", timeSteps.data(), States.current.data(), timeSteps.size());
      ImPlot::PlotLine("Rotational speed", timeSteps.data(), States.rotSpeed.data(), timeSteps.size());
      ImPlot::PlotLine("Rotation", timeSteps.data(), States.rotation.data(), timeSteps.size());
      ImPlot::PlotLine("Input voltage", timeSteps.data(), States.inputVoltage.data(), timeSteps.size());
    }

    ImPlot::EndPlot();
  }
  ImGui::End();
}

void WindowManager::processEvents(bool& done) {
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

// Getters and setters
void WindowManager::addTimeStep(float timeStep) { timeSteps.push_back(timeStep); }
void WindowManager::addState(CircutState state) {
  States.resistorVoltage.push_back(state.ResistorVoltage);
  States.inductorVoltage.push_back(state.inductorVoltage);
  States.motorVoltage.push_back(state.motorVoltage);
  States.current.push_back(state.current);
  States.rotSpeed.push_back(state.rot_speed);
  States.rotation.push_back(state.rotation);
  States.inputVoltage.push_back(state.InputVoltage);
}

CircutParameters WindowManager::getParams() const { return params; }
WindowOptions    WindowManager::getOptions() const { return options; }

InputShape WindowManager::getInputShape() const {
  if (options.inputType == 0)
    return InputShape::Harmonic;
  else if (options.inputType == 1)
    return InputShape::Square;
  else if (options.inputType == 2)
    return InputShape::Triangle;
  else if (options.inputType == 3)
    return InputShape::DC;
  assert(false);  // This should never happen
}

void WindowManager::renderParametersWindow(std::function<void(void)> const& resetCallback) {
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(300, ImGui::GetIO().DisplaySize.y));

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
  ImGui::Begin("Left Side Window", nullptr, window_flags);

  const char* inputTypeItems[] = {"Harmonic", "Square", "Triangle", "DC"};
  ImGui::Combo("Input type", &options.inputType, inputTypeItems, IM_ARRAYSIZE(inputTypeItems));
  ImGui::InputFloat("DC offset", &options.offset);
  ImGui::InputFloat("Amplitude", &options.amplitude);
  ImGui::InputFloat("Pulsation", &options.pulse);
  ImGui::InputFloat("Resistance", &params.R);
  ImGui::InputFloat("Inductance", &params.L);
  ImGui::InputFloat("Kt", &params.Kt);
  ImGui::InputFloat("Ke", &params.Ke);
  ImGui::InputFloat("time step", &options.time_step, 0.0, 0.0f, "%.10f");
  ImGui::InputFloat("Momentum", &params.I);
  ImGui::InputFloat("Stiffness", &params.k);
  ImGui::Checkbox("Start simulation", &options.start_simulation);
  ImGui::Checkbox("auto scroll", &options.auto_scroll);
  ImGui::BulletText("Press 'ESC' to exit the program");
  ImGui::BulletText("Currently saved states: %zu", States.inputVoltage.size());
  ImGui::BulletText("Currently saved time steps: %zu", timeSteps.size());
  ImGui::Text("Current values: ");

  if (timeSteps.size() > 0) {
    ImGui::BulletText("current %f", States.current.back());
    ImGui::BulletText("rotational speed %f [rad/s]", States.rotSpeed.back());
    ImGui::BulletText("rotations %f", States.rotation.back());
    ImGui::BulletText("Input voltage %f", States.inputVoltage.back());
    ImGui::BulletText("Resistor voltage %f", States.resistorVoltage.back());
    ImGui::BulletText("Inductor voltage %f", States.inductorVoltage.back());
    ImGui::BulletText("Motor voltage %f", States.motorVoltage.back());
    ImGui::BulletText("Time %f", timeSteps.back());
  }

  if (ImGui::Button("Clear Data")) {
    States.clear();
    timeSteps.clear();
  }

  if (ImGui::Button("Reset")) {
    States.clear();
    timeSteps.clear();
    resetCallback();
  }
  ImGui::End();
}
