#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <cmath>
#include <iostream>
#define M_PI 3.14159265358979323846





float harmonic(uint64_t t, uint64_t w)
{
    return sinf(2*M_PI*t*w);
}
float square(uint64_t t, uint64_t w)
{
    if(sinf(2*M_PI*t*w) >= 0)
    {
        return 1 ;
    }
    else
    {
        return -1;
    }
    
}
float triangle(uint64_t t, uint64_t w)
{
    int deg = (360*t*w); 
    deg = deg%360;

    if(deg>0 and deg<=90)
    {
        return  static_cast<float>(deg/90) ;
    }
    if(deg>90 and deg<=270)
    {
        return static_cast<float>(-deg/90+2) ;
    }
    if(deg>270 and deg<=360)
    {
        return static_cast<float>(deg/90-4) ;
    }
    else return 0;   
}




enum class UpdateType { SteadyState, Transient };

struct CircutParameters {
  uint64_t Resistance{};
  int64_t
      inputVoltage{}; // input voltage may possibly be a callback in the future
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
    

    uint64_t t = 3.0;
    uint64_t w = 1;
    std:: cout << harmonic(t,w) << ' ';
    std:: cout << square(t,w)<< ' ';
    std:: cout << triangle(t,w)<< ' ';


  time_t CurrentTime{};
  time_t deltaTime{};
  CircutManager circutManager;
  auto [state, parameters] = getInitalState();
  circutManager.update(deltaTime);
  return 0;

}
