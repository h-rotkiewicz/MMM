#include <ctime>
#include <functional>
#include <tuple>
#include <vector>

#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include "common.h"
#include "imgui.h"

struct WindowOptions {
  bool  start_simulation = false;
  bool  auto_scroll      = true;
  int   inputType        = 0;
  float pulse            = 1;
  float amplitude        = 1;
  float time_step        = 0.01f;
  float offset           = 0;
};

class WindowManager {
  constexpr static auto background_color = ImVec4(0.55f, 0.55f, 0.60f, 1.00f);

  struct {
    std::vector<decltype(CircutState::ResistorVoltage)> resistorVoltage;
    std::vector<decltype(CircutState::inductorVoltage)> inductorVoltage;
    std::vector<decltype(CircutState::motorVoltage)>    motorVoltage;
    std::vector<decltype(CircutState::current)>         current;
    std::vector<decltype(CircutState::rot_speed)>       rotSpeed;
    std::vector<decltype(CircutState::rotation)>        rotation;
    std::vector<decltype(CircutState::InputVoltage)>    inputVoltage;

    void clear() {
      resistorVoltage.clear();
      inductorVoltage.clear();
      motorVoltage.clear();
      current.clear();
      rotSpeed.clear();
      rotation.clear();
      inputVoltage.clear();
    }
  } States;

  ImGuiIO            io;
  SDL_Window*        window;
  SDL_Renderer*      renderer;
  std::vector<float> timeSteps;
  WindowOptions      options;
  CircutParameters   params;

  ImGuiIO initImgui();
  std::tuple<SDL_Window*, SDL_Renderer*> initBackend();

 public:
  WindowManager();
  void render();
  void renderPlotWindow();

  void renderParametersWindow(std::function<void(void)> const& resetCallback);
  void processEvents(bool& done);
  void newFrame();
  void addTimeStep(float timeStep);
  void addState(CircutState state);

  InputShape       getInputShape() const;
  CircutParameters getParams() const;
  WindowOptions    getOptions() const;
};


