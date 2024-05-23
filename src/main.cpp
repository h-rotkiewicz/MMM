#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <cmath>
#define DEBUG

enum class UpdateType { SteadyState, Transient };
enum class InputShape { Harmonic, Square, Triangle };

auto harmonic = [](time_t t, uint64_t w) -> float {
  return sinf(2*M_PI*t*w);
}; 

auto square = [](uint64_t t, uint64_t w) -> float
{
    if(sinf(2*M_PI*t*w) >= 0)
    {
        return 1 ;
    }
    else
    {
        return -1;
    }

};
auto triangle = [](float t, uint64_t w) -> float
{
    float deg = (360*t*w); 
    deg = (int)deg%360;

    if(deg>0 and deg<=90)
    {
        return ((float)deg/90) ;
    }
    if(deg>90 and deg<=270)
    {
        return ((float)-deg/90+2) ;
    }
    if(deg>270 and deg<=360)
    {
        return ((float)deg/90-4) ;
    }
    else return 0;   
};

using InputTable = std::unordered_map<InputShape, std::variant< decltype(harmonic), decltype(square), decltype(triangle)>>;

struct CircutParameters {
  uint64_t Resistance{};
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
  CircutManager() {
    input.emplace(InputShape::Harmonic, harmonic);
    input.emplace(InputShape::Square, square);
    input.emplace(InputShape::Triangle, triangle);
  }

  UpdateType update(InputShape shape, uint64_t w ,float currentTime,float deltaT) {
    float Voltage = std::visit([=](auto&& func) { return func(w, currentTime); }, input.at(shape));
#ifdef DEBUG 
    std::cout << "Input Voltage: " << Voltage <<std::endl;
#endif
    // TODO: implement the update function
    return UpdateType::SteadyState;
  }

private:
  CircutState state{};
  CircutParameters params{};
  InputTable input{};
};

std::pair<CircutState, CircutParameters> getInitalState() {
  /* The function will probably serve as a callback for the TGUI library,
   * meaning it will collect data entered by a user in a gui and init
   * CircutState and CircutParameters for now it returns sample conditions for
   * ease of development*/
 return {CircutState{0, 0, 0, 0},
          CircutParameters{.Resistance = 1, .inductance = 1, .Kt = 1, .Ke = 2}};
}

int main() {
  float CurrentTime{};
  float deltaTime = 0.01;
  CircutManager circutManager;
  auto [state, parameters] = getInitalState();
  while(1){
  CurrentTime+=deltaTime;
  circutManager.update(InputShape::Harmonic,1,CurrentTime,deltaTime);
  }
  return 0;
}
