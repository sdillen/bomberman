#include "logger.hpp"
#include <cstdarg>
#include <cstdio>
#include <ctime>

LogLevel Logger::s_currentLogLevel = LogLevel::Info;

void Logger::setLevel(LogLevel level) {
  s_currentLogLevel = level;
}

LogLevel Logger::getLevel() {
  return s_currentLogLevel;
}

void Logger::debug(const char *format, ...) {
  va_list args;
  va_start(args, format);
  log(LogLevel::Debug, format, args);
  va_end(args);
}

void Logger::info(const char *format, ...) {
  va_list args;
  va_start(args, format);
  log(LogLevel::Info, format, args);
  va_end(args);
}

void Logger::warn(const char *format, ...) {
  va_list args;
  va_start(args, format);
  log(LogLevel::Warn, format, args);
  va_end(args);
}

void Logger::error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  log(LogLevel::Error, format, args);
  va_end(args);
}

void Logger::log(LogLevel level, const char *format, va_list args) {
  if (level >= s_currentLogLevel) {
    const char *time_str = currentTimeStr();
    printf("[%s] [%s] ", time_str, levelToString(level));
    vprintf(format, args);
    printf("\n");
  }
}

const char *Logger::levelToString(LogLevel level) {
  switch (level) {
  case LogLevel::Debug:
    return "DEBUG";
  case LogLevel::Info:
    return "INFO";
  case LogLevel::Warn:
    return "WARN";
  case LogLevel::Error:
    return "ERROR";
  }
}

const char *Logger::currentTimeStr() {
  static char time_str[20];
  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
  return time_str;
}
