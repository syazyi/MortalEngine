#ifndef LOGGER_H_
#define LOGGER_H_

#include <memory>
#include "Core/Core.h"
#include "spdlog/spdlog.h"

namespace mortal
{
enum class ELoggerLevel : uint8_t{
    Warning,
    Error,
    Info,
    Fatal,
    Trace
};


class MORTAL_API Logger{
public:
    //使用统一的初始化类进行初始化
    Logger();
    ~Logger();
    //这里暂时使用自带的初始化进行测试
    static Logger* InitOnlyTest();

    template<typename... Args>
    void Log(ELoggerLevel level, Args&&... arg){
        switch (level)
        {   
            case ELoggerLevel::Error:
                m_logger->error(std::forward<Args>(arg)...);
                break;
            case ELoggerLevel::Info:
                m_logger->info(std::forward<Args>(arg)...);
                break;
            case ELoggerLevel::Warning:
                m_logger->warn(std::forward<Args>(arg)...);
                break;
            case ELoggerLevel::Trace:
                m_logger->trace(std::forward<Args>(arg)...);
                break;
            default:
                break;
        }
    }
        
private:
    std::shared_ptr<spdlog::logger> m_logger;

};

} // namespace mortal

#endif