#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

LogLevel currentLogLevel = LOG_LEVEL_INFO;

const char *log_level_to_string(LogLevel level) {
  switch (level) {
  case LOG_LEVEL_DEBUG:
    return "DEBUG";
  case LOG_LEVEL_INFO:
    return "INFO";
  case LOG_LEVEL_WARN:
    return "WARN";
  case LOG_LEVEL_ERROR:
    return "ERROR";
  }
}

void log_message(LogLevel level, const char *format, ...) {
  if (level >= currentLogLevel) {
    va_list args;
    va_start(args, format);

    const char *time_str = get_current_time_str();

    printf("[%s] [%s] ", time_str, log_level_to_string(level));
    vprintf(format, args);
    printf("\n");

    va_end(args);
  }
}

const char *get_current_time_str() {
  static char time_str[20];
  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
  return time_str;
}
