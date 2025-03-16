#pragma once
#include <map>
#include <string>
#include <random>
#include <functional>

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

	ReplaceConfig config;
	std::mt19937 rng;
	std::map<std::string, Handler> handlers;
	std::map<std::string, unsigned int> counters;
};