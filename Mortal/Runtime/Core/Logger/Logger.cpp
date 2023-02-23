#include "Logger.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace mortal
{

    Logger::Logger()
    {
        m_logger = spdlog::stdout_color_mt("Mortal");
        m_logger->set_pattern("[%Y-%m-%d %T] %^[%l]%$: %v");
        m_logger->set_level(spdlog::level::trace);
    }

    Logger::~Logger(){
        spdlog::shutdown();
    }

    Logger* Logger::InitOnlyTest()
    {
        static Logger logger;
        return &logger;
    }
    
} // namespace mortal
