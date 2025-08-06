#ifndef LOGGER_H
#define LOGGER_H
#include <Arduino.h>
#include "LogLevels.h"
#include "Config.h"

namespace logger
{

    template <typename T>
    static void print(T msg, enum LogLevel level = LogLevel::Debug)
    {
        // Only print if the local log level is lower (more important) or equal to the global log level
        if (level <= logging::level)
        {
            Serial.print(msg);
        }
    }
    template <typename T>
    static void println(T msg = "\n", enum LogLevel level = LogLevel::Debug)
    {
        print(String(msg) + "\n", level);
    }
    static void println()
    {
        println("");
    }

    static void flush()
    {
        Serial.flush();
    }
}
#endif