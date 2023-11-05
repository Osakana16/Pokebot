#pragma once
#include "behavior.hpp"
#include "bot.hpp"
#include "game.hpp"

#define POKEBOT_PLUGIN_GOLDSRC 0
#define POKEBOT_PLUGIN_SOURCE 1
#define POKEBOT_PLUGIN POKEBOT_PLUGIN_GOLDSRC

namespace pokebot::plugin {
	inline class Pokebot {
	public:
		void OnUpdate() noexcept;
		void AddBot(const std::string& Bot_Name, const common::Team, const common::Model) POKEBOT_DEBUG_NOEXCEPT;
	} pokebot_plugin{};
}