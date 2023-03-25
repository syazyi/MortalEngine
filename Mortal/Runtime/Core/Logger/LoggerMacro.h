#ifndef LOGGERMACRO_H_
#define LOGGERMACRO_H_

#include "Logger.h"
	
#define MORTAL_LOG_WARN(...)	mortal::Logger::InitOnlyTest()->Log(mortal::ELoggerLevel::Warning, __VA_ARGS__);
#define MORTAL_LOG_ERROR(...)	mortal::Logger::InitOnlyTest()->Log(mortal::ELoggerLevel::Error, __VA_ARGS__);
#define MORTAL_LOG_INFO(...)	mortal::Logger::InitOnlyTest()->Log(mortal::ELoggerLevel::Info, __VA_ARGS__);
#define MORTAL_LOG_TRACE(...)	mortal::Logger::InitOnlyTest()->Log(mortal::ELoggerLevel::Trace, __VA_ARGS__);

#endif

