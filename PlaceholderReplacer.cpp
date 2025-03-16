#include "PlaceholderReplacer.h"
#include <stack>
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <ctime>

PlaceholderReplacer::PlaceholderReplacer(const ReplaceConfig& cfg) : config(cfg) {
	// 初始化随机数生成器
	std::random_device rd;
	rng.seed(rd());

	// 注册内置处理器
	registerHandlers();
}

std::string PlaceholderReplacer::replace(const std::string& input) {
	std::string result = input;
	bool changed;

	// 循环处理直到没有更多占位符
	do {
		changed = false;
		std::stack<size_t> st;
		size_t pos = 0;

		// 手动解析嵌套结构
		while (pos < result.length()) {
			if (result[pos] == '$' && pos + 1 < result.length() && result[pos + 1] == '{') {
				st.push(pos);
				pos += 2;
			}
			else if (result[pos] == '}' && !st.empty()) {
				size_t start = st.top();
				st.pop();

				// 提取完整占位符 ${...}
				std::string full = result.substr(start, pos - start + 1);
				std::string content = result.substr(start + 2, pos - start - 2);

				// 处理占位符
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
					replacement = full; // 保持原样
				}

				// 执行替换
				result.replace(start, pos - start + 1, replacement);
				pos = start + replacement.length();
				changed = true;

			}
			else {
				++pos;
			}
		}
	} while (changed); // 需要多次处理嵌套内容

	return result;
}
void PlaceholderReplacer::registerHandlers() {
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
	handlers["PROJECT"] = [this](const std::string&) {
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
	handlers["COUNTER"] = [this](const std::string& name) {
		std::string key = name.empty() ? "" : name;
		unsigned int& count = counters[key];
		return key + std::to_string(++count);
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

std::string PlaceholderReplacer::generateRandom(int length) {
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

std::string PlaceholderReplacer::generateUUID() {
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