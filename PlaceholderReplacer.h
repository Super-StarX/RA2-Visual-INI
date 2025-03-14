﻿#include <iostream>
#include <string>
#include <regex>
#include <chrono>
#include <random>
#include <ctime>
#include <map>
#include <functional>
#include <cstdlib>

struct ReplaceConfig {
	std::string projectName = "";
};

class PlaceholderReplacer {
	ReplaceConfig config;
	std::mt19937 rng;

	// 变量处理函数类型
	using Handler = std::function<std::string(const std::string&)>;

	// 注册的处理函数映射表
	std::map<std::string, Handler> handlers;

public:
	PlaceholderReplacer(const ReplaceConfig& cfg = {}) : config(cfg) {
		// 初始化随机数生成器
		std::random_device rd;
		rng.seed(rd());

		// 注册内置处理器
		registerHandlers();
	}

	std::string replace(const std::string& input) {
		std::regex re(R"(\$\{([^}]+)\})");
		std::sregex_iterator it(input.begin(), input.end(), re);
		std::sregex_iterator end;

		std::string result = input;
		size_t pos_offset = 0;

		for (; it != end; ++it) {
			std::smatch match = *it;
			std::string full_match = match.str();
			std::string content = match[1].str();

			// 分解变量和参数
			size_t colon_pos = content.find(':');
			std::string var = content.substr(0, colon_pos);
			std::string arg = (colon_pos != std::string::npos) ?
				content.substr(colon_pos + 1) : "";

			std::string replacement;

			if (handlers.find(var) != handlers.end()) {
				replacement = handlers[var](arg);
			}
			else if (var.find("RANDOM_") == 0) {
				int length = std::stoi(var.substr(7));
				replacement = generateRandom(length);
			}
			else {
				replacement = full_match; // 保持未知变量不变
			}

			// 替换并调整偏移量
			size_t pos = match.position() + pos_offset;
			result.replace(pos, full_match.length(), replacement);
			pos_offset += replacement.length() - full_match.length();
		}

		return result;
	}

private:
	void registerHandlers() {
		// 时间相关
		handlers["TIMESTAMP"] = [this](const std::string&) {
			auto now = std::chrono::system_clock::now();
			return std::to_string(
				std::chrono::duration_cast<std::chrono::milliseconds>(
					now.time_since_epoch()).count());
		};

		handlers["DATETIME"] = [this](const std::string&) {
			auto now = std::chrono::system_clock::now();
			std::time_t t = std::chrono::system_clock::to_time_t(now);

			// 使用安全版本的localtime
			std::tm tm_buf;
			localtime_s(&tm_buf, &t);

			char buf[32];
			std::strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tm_buf);
			return std::string(buf);
		};

		handlers["DATE"] = [this](const std::string& format) {
			auto now = std::chrono::system_clock::now();
			std::time_t time = std::chrono::system_clock::to_time_t(now);

			std::tm tm_buf;
			localtime_s(&tm_buf, &time);

			std::ostringstream oss;
			oss << std::put_time(&tm_buf, format.empty() ? "%Y%m%d%H%M%S" : format.c_str());
			return oss.str();
		};

		// 随机值
		handlers["RANDOM"] = [this](const std::string& arg) {
			int length = arg.empty() ? 8 : std::stoi(arg);
			return generateRandom(length);
		};

		handlers["UUID"] = [this](const std::string&) {
			return generateUUID();
		};

		// 项目相关
		handlers["PROJECT_NAME"] = [this](const std::string&) {
			return config.projectName;
		};

		// 环境变量
		handlers["ENV"] = [this](const std::string& var) {
			char* value = nullptr;
			size_t len = 0;
			_dupenv_s(&value, &len, var.c_str());

			std::string result;
			if (value) {
				result = value;
				free(value);
			}
			return result;
		};

		// 计数器
		handlers["COUNTER"] = [this](const std::string&) {
			static unsigned int counter = 0;
			return std::to_string(++counter);
		};

		handlers["HASH"] = [](const std::string& input) {
			// 使用简单的 djb2 哈希算法
			unsigned long hash = 5381;
			for (char c : input) {
				hash = ((hash << 5) + hash) + c; // hash * 33 + c
			}

			// 转换为16进制字符串
			std::stringstream ss;
			ss << std::hex << std::setw(8) << std::setfill('0') << hash;
			return ss.str();
		};
	}

	std::string generateRandom(int length) {
		static const std::string chars =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";

		std::uniform_int_distribution<> dist(0, chars.size() - 1);
		std::string result;
		for (int i = 0; i < length; ++i) {
			result += chars[dist(rng)];
		}
		return result;
	}

	std::string generateUUID() {
		// 简单实现，生产环境建议使用专用库
		std::string uuid(36, '-');
		std::uniform_int_distribution<int> dist(0, 15);
		auto gen = [&] { return dist(rng); };

		for (size_t i = 0; i < 36; ++i) {
			if (i == 8 || i == 13 || i == 18 || i == 23) continue;
			uuid[i] = "0123456789abcdef"[gen()];
		}
		uuid[14] = '4'; // UUID version 4
		uuid[19] = "89ab"[gen() % 4]; // variant
		return uuid;
	}
};

// 使用示例
int main() {
	PlaceholderReplacer replacer({ .projectName = "AwesomeProject" });

	std::cout << replacer.replace("timestamp_${TIMESTAMP}") << "\n";
	std::cout << replacer.replace("build_${DATE:%%Y%%m%%d}") << "\n"; // 注意转义%
	std::cout << replacer.replace("temp_${RANDOM_12}") << "\n";
	std::cout << replacer.replace("${PROJECT_NAME}_backup_${DATETIME}") << "\n";
	std::cout << replacer.replace("Counter: ${COUNTER}, ${COUNTER}") << "\n";
	std::cout << replacer.replace("Host: ${HOSTNAME}") << "\n";
	std::cout << replacer.replace("Path: ${ENV:HOME}") << "\n";
}