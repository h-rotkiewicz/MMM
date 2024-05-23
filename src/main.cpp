#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <cmath>
#include <iostream>
#define pi 3.14





float harmonic(float t, float w)
{
    return sinf(2*pi*t*w);
}
float square(float t, float w)
{
    if(sinf(2*pi*t*w) >= 0)
    {
        return 1 ;
    }
    else
    {
        return -1;
    }
    
}
float triangle(float t, float w)
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

bool runExample(tgui::BackendGui& gui)
{
    return true;
}

int main()
{

float dt = 0.001; 
float Kt  ;
float Ke ;
float R ;
float L ;
float I ;
float em;
float ut;
float phi;

float t = 3.25;
float w = 1;
std:: cout << harmonic(t,w) << ' ';
std:: cout << square(t,w)<< ' ';
std:: cout << triangle(t,w)<< ' ';

    
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "TGUI example");
    tgui::Gui gui(window);
    if(runExample(gui))
        gui.mainLoop();

}
