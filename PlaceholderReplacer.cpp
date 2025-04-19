#include "PlaceholderReplacer.h"
#include <stack>
#include <regex>
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
				std::string arg = (colon_pos != std::string::npos) ? content.substr(colon_pos + 1) : "";

				std::string replacement = handleVariable(var, arg);
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

std::string PlaceholderReplacer::replaceManually(const std::string& jsonStr, const std::string& target, const std::string& replacement) {
	std::string result;
	size_t start = 0, pos;

	// 单次遍历并直接构建结果（无需记录所有位置）
	result.reserve(jsonStr.size() + 32); // 基础预分配减少扩容
	while ((pos = jsonStr.find(target, start)) != std::string::npos) {
		result.append(jsonStr.data() + start, pos - start);
		result.append(replacement);
		start = pos + target.size();
	}
	result.append(jsonStr.data() + start, jsonStr.size() - start);

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
	handlers["COUNTER"] = [this](const std::string& arg) {
		return handleCounter(arg);
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

	std::uniform_int_distribution<> dist(0, static_cast<int>(chars.size()) - 1);
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

std::string PlaceholderReplacer::handleVariable(const std::string& var, const std::string& params) {
	if (var == "COUNTER") {
		return handleCounter(params);
	}
	else if (var.find("RANDOM_") == 0) {
		int length = std::stoi(var.substr(7));
		return generateRandom(length);
	}
	else if (handlers.count(var)) {
		return handlers[var](params);
	}
	return ""; // 未知变量保持原样
}

// 添加计数器处理函数
std::string PlaceholderReplacer::handleCounter(const std::string& params) {
	// 默认计数器配置
        // 默认计数器配置
	CounterState state;
	parseParams(params, state);

	// 获取或初始化计数器
	CounterState& counter = counters[state.format.empty() ? "default" : state.format];
	if (counter.value == 0) { // 初始化默认值
		counter = state;
	}

	// 生成计数器值
	counter.value += counter.step;
	unsigned int current = counter.value;

	// 格式化输出
	return formatValue(current, counter);
}

std::string PlaceholderReplacer::formatValue(unsigned int value, const CounterState& state) {
	if (!state.format.empty()) {
		char buffer[256];
		snprintf(buffer, sizeof(buffer), state.format.c_str(), value);
		return buffer;
	}

	// 默认格式化逻辑
	std::stringstream ss;
	ss << std::setfill('0') << std::setw(state.width);

	if (state.base == "hex")
		ss << std::hex << value;
	else if (state.base == "oct")
		ss << std::oct << value;
	else
		ss << std::dec << value;

	return ss.str();
}
std::string PlaceholderReplacer::formatDefault(unsigned int value, const CounterState& state) {
	std::stringstream ss;
	if (state.base == "hex") {
		ss << std::hex << std::setw(state.width) << std::setfill('0') << value;
	}
	else if (state.base == "oct") {
		ss << std::oct << std::setw(state.width) << std::setfill('0') << value;
	}
	else {
		ss << std::dec << std::setw(state.width) << std::setfill('0') << value;
	}
	return ss.str();
}

void PlaceholderReplacer::parseParams(const std::string& params, CounterState& state) {
	std::istringstream iss(params);
	std::string param;
	while (std::getline(iss, param, ',')) {
		size_t eq_pos = param.find('=');
		if (eq_pos == std::string::npos) continue;

		std::string key = param.substr(0, eq_pos);
		std::string value = param.substr(eq_pos + 1);

		if (key == "step") state.step = std::stoi(value);
		else if (key == "width") state.width = std::stoi(value);
		else if (key == "base") state.base = value;
		else if (key == "format") state.format = value;
	}
}

std::string PlaceholderReplacer::parseCounterFormat(const std::string& format, unsigned int value) {
	std::string temp = format;

	// 替换内部占位符
	size_t pos = 0;
	while ((pos = temp.find("{", pos)) != std::string::npos) {
		size_t end = temp.find("}", pos);
		if (end == std::string::npos) break;

		std::string placeholder = temp.substr(pos + 1, end - pos - 1);
		std::string replacement;

		// 处理特殊标记
		if (placeholder == "VALUE") {
			replacement = std::to_string(value);
		}
		else {
			// 递归处理其他占位符
			replacement = replace("${" + placeholder + "}");
		}

		temp.replace(pos, end - pos + 1, replacement);
		pos += replacement.length();
	}

	// 处理格式说明符
	char buffer[256];
	snprintf(buffer, sizeof(buffer), temp.c_str(), value);
	return buffer;
}