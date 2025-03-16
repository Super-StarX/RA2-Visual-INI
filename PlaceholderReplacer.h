#pragma once
#include <iostream>
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