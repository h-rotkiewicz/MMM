#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>

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
