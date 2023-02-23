#include "Mortal.h"
#include "spdlog/spdlog.h"

int main(int argc, char** argv){
    mortal::Application app;
    MORTAL_LOG_INFO("hello!");
    MORTAL_LOG_WARN("Warning!");
}