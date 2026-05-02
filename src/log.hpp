#pragma once

enum class LogLevel {
  Debug = 0,
  Info,
  Warn,
  Error,
};

extern LogLevel currentLogLevel;

const char *log_level_to_string(LogLevel level);

void log_message(LogLevel level, const char *format, ...);

#define LOG_DEBUG(format, ...) log_message(LogLevel::Debug, format, __VA_ARGS__)
#define LOG_INFO(format, ...) log_message(LogLevel::Info, format, __VA_ARGS__)
#define LOG_WARN(format, ...) log_message(LogLevel::Warn, format, __VA_ARGS__)
#define LOG_ERROR(format, ...) log_message(LogLevel::Error, format, __VA_ARGS__)

const char *get_current_time_str();


