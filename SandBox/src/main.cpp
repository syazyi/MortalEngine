#include "Mortal.h"
#include "spdlog/spdlog.h"
#include "Window/WindowsWindow.h"
int main(int argc, char** argv){
    mortal::Application app(new mortal::WindowsWindow("mortal", 1600, 900));

    app.Run();
}