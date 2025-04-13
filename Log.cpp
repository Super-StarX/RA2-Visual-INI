#include "Log.h"

namespace Log {
	void Init() {
		spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
		spdlog::set_level(spdlog::level::trace);
		spdlog::rotating_logger_mt("VILogger", "Logs/VI.log", 1024 * 1024 * 10, 5); // 日志最大记录10MB, 最多备份5份日志
	}

	std::shared_ptr<spdlog::logger> GetLogger() {
		return spdlog::get("VILogger");
	}
}