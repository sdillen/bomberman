#pragma once
#include <cstdarg>

enum class LogLevel {
  Debug = 0,
  Info,
  Warn,
  Error,
};

class Logger {
public:
  static void setLevel(LogLevel level);
  static LogLevel getLevel();

  static void debug(const char *format, ...);
  static void info(const char *format, ...);
  static void warn(const char *format, ...);
  static void error(const char *format, ...);

private:
  static LogLevel s_currentLogLevel;

  static void log(LogLevel level, const char *format, va_list args);
  static const char *levelToString(LogLevel level);
  static const char *currentTimeStr();
};

#define LOG_DEBUG(format, ...) Logger::debug(format, __VA_ARGS__)
#define LOG_INFO(format, ...) Logger::info(format, __VA_ARGS__)
#define LOG_WARN(format, ...) Logger::warn(format, __VA_ARGS__)
#define LOG_ERROR(format, ...) Logger::error(format, __VA_ARGS__)
