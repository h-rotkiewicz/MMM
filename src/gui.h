#include <ctime>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include "common.h"
#include "imgui.h"

class Window {
  constexpr static auto background_color = ImVec4(0.55f, 0.55f, 0.60f, 1.00f);

  ImGuiIO                  io;
  SDL_Window*              window;
  SDL_Renderer*            renderer;
  int                      inputType = 0;
  float                    time_step = 0.01f;
  CircutParameters         params;
  bool                     start_simulation = false;
  bool                     auto_scroll      = true;
  float                    width{};
  float                    amplitude{};
  std::vector<CircutState> states;
  std::vector<float>       timeSteps;

  std::tuple<SDL_Window*, SDL_Renderer*> init_backend();
  auto                                   init_imgui();

 public:
  Window();
  void render();
  void render_parameters_window();
  void render_plot_window();
  void process_events(bool& done);
  void newFrame();
  void add_timeStep(float timeStep);
  void add_state(CircutState state);

  InputShape       getInputShape() const;
  CircutParameters getParams() const;
  float            getWidth() const;
  float            get_amplitude() const;
  bool             is_simulation_started() const;
};
