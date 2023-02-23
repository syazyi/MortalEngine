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
    //ʹ��ͳһ�ĳ�ʼ������г�ʼ��
    Logger();
    ~Logger();
    //������ʱʹ���Դ��ĳ�ʼ�����в���
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