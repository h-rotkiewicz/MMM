#include <tuple>
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include "imgui.h"
class Window {
  constexpr static auto background_color = ImVec4(0.55f, 0.55f, 0.60f, 1.00f);
  ImGuiIO	io;
  SDL_Window*	window;
  SDL_Renderer* renderer;
  std::tuple<SDL_Window*, SDL_Renderer*> init_backend();
  auto init_imgui();
 public:
  Window();
  void render_imgui_window();
  void process_events(bool& done);
  void render();
};

