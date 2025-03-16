#pragma once
#include <map>
#include <string>
#include <random>
#include <functional>

struct CounterState {
	unsigned int value = 0;
	unsigned int step = 1;
	unsigned int width = 0;
	std::string format;
	std::string base = "dec";
};

struct ReplaceConfig {
	std::string projectName = "";
};

class PlaceholderReplacer {
public:
	PlaceholderReplacer(const ReplaceConfig& cfg = {});
	std::string replace(const std::string& input);

private:
	using Handler = std::function<std::string(const std::string&)>;

	void registerHandlers();
	std::string generateRandom(int length);
	std::string generateUUID();
	std::string handleCounter(const std::string& params);
	std::string formatDefault(unsigned int value, const CounterState& state);
	std::string parseCounterFormat(const std::string& format, unsigned int value);

	ReplaceConfig config;
	std::mt19937 rng;
	std::map<std::string, Handler> handlers;
	std::map<std::string, CounterState> counters;
};