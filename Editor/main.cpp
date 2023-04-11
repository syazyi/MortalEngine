#include <map>
#include "Mortal.h"
#include "editor/editor.h"
#include "spdlog/spdlog.h"
#include "Window/WindowsWindow.h"
#include "ECS/ecs.h"
#include "ECS/component.h"


int main(int argc, char** argv){
    mortal::Editor app(new mortal::WindowsWindow("mortal", 1600, 900));
    mortal::Command command;
    command.spawn<mortal::Position>(mortal::Position{1.0f, 2.0f, 3.0f});

    app.Run();
}