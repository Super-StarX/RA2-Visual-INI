#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/logger.h>

#define LOG_DEBUG(...) Log::GetLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...) Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) Log::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) Log::GetLogger()->critical(__VA_ARGS__)

namespace Log {
	void Init();
	std::shared_ptr<spdlog::logger> GetLogger();
}
