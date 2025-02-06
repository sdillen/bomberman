#ifndef LOG_H
#define LOG_H

typedef enum {
  LOG_LEVEL_DEBUG = 0,
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERROR,
} LogLevel;

extern LogLevel currentLogLevel;

const char *log_level_to_string(LogLevel level);

void log_message(LogLevel level, const char *format, ...);

#define LOG_DEBUG(format, ...) log_message(LOG_LEVEL_DEBUG, format, __VA_ARGS__)
#define LOG_INFO(format, ...) log_message(LOG_LEVEL_INFO, format, __VA_ARGS__)
#define LOG_WARN(format, ...) log_message(LOG_LEVEL_WARN, format, __VA_ARGS__)
#define LOG_ERROR(format, ...) log_message(LOG_LEVEL_ERROR, format, __VA_ARGS__)

const char *get_current_time_str();

#endif // LOG_H
