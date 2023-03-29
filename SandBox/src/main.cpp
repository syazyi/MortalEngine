#include "Mortal.h"
#include "sandbox.h"
#include "spdlog/spdlog.h"
#include "Window/WindowsWindow.h"

int main(int argc, char** argv){
    mortal::SandBox app(new mortal::WindowsWindow("mortal", 1600, 900));

    app.Run();
}