#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <cmath>

enum class UpdateType { SteadyState, Transient };

struct CircutParameters {
  uint64_t Resistance{};
  uint64_t
      inputVoltage{}; // input voltage may possibly be a callback in the future
  uint64_t inductance{};
  uint64_t Kt{};
  uint64_t Ke{};
};

struct CircutState {
  uint64_t ResistorVoltage{};
  uint64_t inductorVoltage{};
  uint64_t motorVoltage{};
  uint64_t current{};
};

class CircutManager {
public:
  UpdateType update(time_t deltaT) {
    // TODO: implement the update function
    return UpdateType::SteadyState;
  }

private:
  CircutState state{};
  CircutParameters params{};
};

std::pair<CircutState, CircutParameters> getInitalState() {
  /* The function will probably serve as a callback for the TGUI library,
   * meaning it will collect data entered by a user in a gui and init
   * CircutState and CircutParameters for now it returns sample conditions for
   * ease of development*/
  return {
      CircutState{0, 0, 0, 0},
      CircutParameters{.Resistance = 1, .inputVoltage = 5, .inductance = 1, .Kt = 1, .Ke = 2 }};
}

int main() {
  time_t CurrentTime{};
  time_t deltaTime{};
  CircutManager circutManager;
  auto [state, parameters] = getInitalState();
  circutManager.update(deltaTime);
  return 0;
}
