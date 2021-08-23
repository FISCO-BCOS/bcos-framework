#pragma once

#include "servant/TarsLogger.h"
#include <iostream>

#define LOG_BADGE(_NAME) "[" << (_NAME) << "]"
#define LOG_TYPE(_TYPE) (_TYPE) << "|"
#define LOG_DESC(_DESCRIPTION) (_DESCRIPTION)
#define LOG_KV(_K, _V) "," << (_K) << "=" << (_V)

#define TRACE TarsRollLogger::DEBUG_LOG
#define DEBUG TarsRollLogger::DEBUG_LOG
#define INFO TarsRollLogger::INFO_LOG
#define WARNING TarsRollLogger::WARN_LOG
#define ERROR TarsRollLogger::ERROR_LOG
#define FATAL TarsRollLogger::ERROR_LOG

template <class Logger>
class EndlHelper
{
public:
    EndlHelper(Logger& logger) : m_logger(logger) {}

    template <class Message>
    EndlHelper& operator<<(Message message)
    {
        return m_logger << message;
    }

    ~EndlHelper() { m_logger << std::endl; }

private:
    Logger& m_logger;
};

#define BCOS_LOG(level)        \
    if (LOG->IsNeedLog(level)) \
    EndlHelper(LOG->log(level))
#define BCOS_STAT_LOG(level)   \
    if (LOG->IsNeedLog(level)) \
    EndlHelper(LOG->log(level))