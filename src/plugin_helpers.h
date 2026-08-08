#pragma once

#include "plugin_interface.h"

// Access the global plugin self pointer (defined in plugin.cpp).
IPluginSelf* GetSelf();

// Convenience logging macros — no-op safely if the plugin self pointer is null.
#define LOG_TRACE(format, ...) if (auto s = GetSelf()) s->logger->Trace(s, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) if (auto s = GetSelf()) s->logger->Debug(s, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...)  if (auto s = GetSelf()) s->logger->Info (s, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)  if (auto s = GetSelf()) s->logger->Warn (s, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) if (auto s = GetSelf()) s->logger->Error(s, format, ##__VA_ARGS__)
