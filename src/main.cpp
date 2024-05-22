#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <cmath>
#include <iostream>
#define pi 3.14

bool DEBUG = true; // odpala funkcje do debugowania

float dt = 0.001; //
float Kt  ;
float Ke ;
float R ;
float L ;
float I ;
float em;
float ut;
float phi;

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
    float deg = (360*t*w); 
    if((360*t*w)>360)
    {
        deg-=360;
    }

    if(deg>0 and deg<=90)
    {
        return 2*t*w*2 ;
    }
    if(deg>90 and deg<=270)
    {
        return -2*t*w*2+1 ;
    }
    if(deg>270 and deg<=360)
    {
        return 2*t*w*2-1 ;
    }   
}

bool runExample(tgui::BackendGui& gui)
{
    return true;
}

int main()
{
    
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "TGUI example");
    tgui::Gui gui(window);
    if(runExample(gui))
        gui.mainLoop();

}
