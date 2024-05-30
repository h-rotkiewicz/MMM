#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <cmath>
#include <numbers>
#define M_PI std::numbers::pi
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
  uint64_t R{}; //resistance
  uint64_t L{}; //inductance
  uint64_t Kt{};
  uint64_t Ke{};
  uint64_t I{}; //Momentum
  uint64_t k{}; //spring
};

struct CircutState {
  float ResistorVoltage{};
  float inductorVoltage{};
  float motorVoltage{};
  float current{};
  float rot_speed{};
};

class CircutManager {
public:
  CircutManager() {
    input.emplace(InputShape::Harmonic, harmonic);
    input.emplace(InputShape::Square, square);
    input.emplace(InputShape::Triangle, triangle);
  }
  void init(CircutState state_init, CircutParameters params_init)
  {
    state = state_init;
    params = params_init;
  };

  UpdateType update(InputShape shape, uint64_t w ,float currentTime,float deltaT) {
    float input_Voltage = 10;
    //std::visit([=](auto&& func) { return func(w, currentTime); }, input.at(shape));

    float di = (input_Voltage*deltaT)/params.L-(params.R*state.current*deltaT)/params.L-(params.Ke*state.rot_speed*deltaT/params.L);
    float dw = (params.Kt*state.current*deltaT)/params.I + (params.k*state.rot_speed*deltaT)/params.I;

    state.rot_speed += dw;
    state.current += di;

    state.inductorVoltage = params.L*di/deltaT;
    state.motorVoltage = params.Ke*dw/deltaT;
    state.ResistorVoltage = state.current*params.R;


#ifdef DEBUG 
    std::cout << "Input Voltage: " << input_Voltage <<std::endl;
    std::cout << "Circuit Current: " << state.current <<std::endl;
    std::cout << "Rotational Speed: " << state.rot_speed <<std::endl;
#endif
   
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
 return {CircutState{.ResistorVoltage =0, .inductorVoltage = 0, .motorVoltage = 0, .current = 0, .rot_speed = 0},
          CircutParameters{.R = 1, .L = 1, .Kt = 1, .Ke = 2, .I = 1, .k = 1}};
}




int main() {
  CircutManager circutManager;
  const float deltaTime = 0.01;
  float CurrentTime = 0;
  
  auto [state, parameters] = getInitalState();
  circutManager.init(state, parameters);
  for (size_t i = 0; i < 500; i++)
  {
  CurrentTime = CurrentTime + deltaTime ; 
  #ifdef DEBUG 
    std::cout << "CurrentTime: " << CurrentTime <<std::endl;
   
 #endif
  circutManager.update(InputShape::Harmonic,1,CurrentTime,deltaTime);
  }
  
  return 0;

}
