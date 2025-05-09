#include "bot/manager.hpp"

import pokebot.util.random;

namespace pokebot::bot {
	namespace {
		bool IsTerrorist(const BotPair& target) noexcept { return target.second.JoinedTeam() == common::Team::T; }
		bool IsCounterTerrorist(const BotPair& target) noexcept { return target.second.JoinedTeam() == common::Team::CT; }
	}

	Manager::Manager() :
		troops{
			Troops(IsTerrorist, common::Team::T),
			Troops(IsCounterTerrorist, common::Team::CT)
	} {

	}

	void Manager::OnNewRoundPreparation() POKEBOT_NOEXCEPT {
		for (auto& bot : bots) {
			bot.second.OnNewRound();
		}
		c4_origin = std::nullopt;
		initialization_stage = InitializationStage::Player_Action_Ready;
		round_started_timer.SetTime(1.0f);
	}

	void Manager::OnNewRoundReady() POKEBOT_NOEXCEPT {
		if (!round_started_timer.IsRunning() && initialization_stage != InitializationStage::Player_Action_Ready) {
			return;
		}

		for (auto& troop : troops) {
			troop.DecideStrategy(&bots, game::game.GetMapFlag());
			troop.Command(&bots);
			
			for (auto& platoon : troop) {
				platoon.DecideStrategy(&bots, game::game.GetMapFlag());
				platoon.Command(&bots);
			}
		}
		initialization_stage = InitializationStage::Completed;
	}

	bool Manager::IsExist(const std::string_view& Bot_Name) const POKEBOT_NOEXCEPT {
		auto it = bots.find(Bot_Name.data());
		return (it != bots.end());
	}

	void Manager::Assign(const std::string_view& Bot_Name, Message message) POKEBOT_NOEXCEPT {
		if (auto target = Get(Bot_Name.data()); target != nullptr) {
			target->start_action = message;
		}
	}

	void Manager::OnDied(const std::string_view& Bot_Name) POKEBOT_NOEXCEPT {
		auto bot = Get(Bot_Name.data());
		if (bot != nullptr) {
			bot->current_weapon = game::Weapon::None;
			bot->goal_queue.Clear();
		}
	}

	void Manager::OnDamageTaken(const std::string_view Bot_Name, const edict_t* Inflictor, const int Damage, const int Armor, const int Bit) POKEBOT_NOEXCEPT {
		if (decltype(auto) target = Get(Bot_Name.data()); target->Health() <= 0) {
			OnDied(Bot_Name.data());
		} else {
			// TODO: Send the event message for a bot.
		}
	}

	void Manager::OnJoinedTeam(const std::string_view&) POKEBOT_NOEXCEPT {

	}

	void Manager::OnChatRecieved(const std::string_view&) POKEBOT_NOEXCEPT {

	}

	void Manager::OnTeamChatRecieved(const std::string_view&) POKEBOT_NOEXCEPT {

	}

	void Manager::OnRadioRecieved(const std::string_view& Sender_Name, const std::string_view& Radio_Sentence) POKEBOT_NOEXCEPT {
		const auto Leader_Client = game::game.clients.Get(Sender_Name.data());

		radio_message.team = Leader_Client->GetTeam();
		radio_message.sender = Sender_Name.data();
		radio_message.message = Radio_Sentence.data();

		auto& troop = troops[static_cast<int>(radio_message.team) - 1];

		auto createLeaderPlatoon = [&]() -> PlatoonID {
			return troop.CreatePlatoon([&](const BotPair& Pair) { return common::Distance(Pair.second.Origin(), Leader_Client->origin) <= 1000.0f; });
		};

		if (radio_message.message == "#Cover_me") {
			radio_message.platoon = createLeaderPlatoon();
		} else if (radio_message.message == "#Follow_me") {
			radio_message.platoon = createLeaderPlatoon();
		} else if (radio_message.message == "#Stick_together_team") {
			radio_message.platoon = createLeaderPlatoon();
		}
	}

	void Manager::Insert(util::PlayerName bot_name, const common::Team team, const common::Model model, const bot::Difficult Assigned_Diffcult) POKEBOT_NOEXCEPT {
		if (auto spawn_result = game::game.Spawn(bot_name.c_str()); std::get<bool>(spawn_result)) {
			bot_name = std::get<util::PlayerName>(spawn_result).c_str();
			auto insert_result = bots.insert({ bot_name.c_str(), Bot(bot_name.c_str(), team, model) });
			assert(insert_result.second);
		}
	}

	void Manager::Update() POKEBOT_NOEXCEPT {
		if (bots.empty())
			return;

		OnNewRoundReady();
		if (game::game.IsCurrentMode(game::MapFlags::Demolition)) {
			if (!c4_origin.has_value()) {
				if (bomber_name.empty()) {
					// When the bomb is dropped:
					// 
					if (backpack != nullptr) {

					} else {
						edict_t* dropped_bomb{};
						while ((dropped_bomb = common::FindEntityByClassname(dropped_bomb, "weaponbox")) != NULL) {
							if (std::string_view(STRING(dropped_bomb->v.model)) == "models/w_backpack.mdl") {
								backpack = dropped_bomb;
								terrorist_troop.DecideStrategyFromRadio(&bots, RadioMessage{ .team=common::Team::T, .message="#Take_Bomb"  });
								break;
							}
						}
					}
				}

				edict_t* c4{};
				while ((c4 = common::FindEntityByClassname(c4, "grenade")) != nullptr) {
					if (std::string_view(STRING(c4->v.model)) == "models/w_c4.mdl") {
						c4_origin = c4->v.origin;
						break;
					}
				}

				if (c4_origin.has_value()) {
					OnBombPlanted();
				}
			}
		}


		for (auto& bot : bots) {
			bot.second.Run();
		}


		if (!radio_message.sender.empty()) {
			auto followers = (bots | std::views::filter([&](const std::pair<util::PlayerName, Bot>& Pair) -> bool { return radio_message.team == Pair.second.JoinedTeam() && radio_message.sender != Pair.first; }));
			for (auto& follower : followers) {
				assert(follower.first != radio_message.sender);
				follower.second.OnRadioRecieved(radio_message.sender.c_str(), radio_message.message.c_str());
			}

			if (radio_message.platoon.has_value()) {
				for (auto& follower : followers) {
					assert(follower.first != radio_message.sender);
					follower.second.platoon = *radio_message.platoon;
				}
				auto& platoon = troops[static_cast<int>(radio_message.team) - 1][*radio_message.platoon];
				platoon.DecideStrategyFromRadio(&bots, radio_message);
				platoon.Command(&bots);
			}

			radio_message.sender.clear();
			radio_message.platoon = std::nullopt;
			radio_message.message.clear();
		}
	}


	Bot* const Manager::Get(const std::string_view& Bot_Name) POKEBOT_NOEXCEPT {
		auto bot_iterator = bots.find(Bot_Name.data());
		return (bot_iterator != bots.end() ? &bot_iterator->second : nullptr);
	}

	void Manager::Kick(const std::string_view& Bot_Name) POKEBOT_NOEXCEPT {
		(*g_engfuncs.pfnServerCommand)(std::format("kick \"{}\"", Bot_Name).c_str());
	}

	void Manager::Remove(const std::string_view& Bot_Name) POKEBOT_NOEXCEPT {
		if (auto bot_iterator = bots.find(Bot_Name.data()); bot_iterator != bots.end()) {
			bots.erase(Bot_Name.data());
		}
	}

	void Manager::OnBombPlanted() POKEBOT_NOEXCEPT {
		for (auto& bot : bots) {
			bot.second.OnBombPlanted();
		}
	}

	void Manager::OnBombPickedUp(const std::string_view& Client_Name) POKEBOT_NOEXCEPT {
		bomber_name = Client_Name.data();
		backpack = nullptr;
	}

	void Manager::OnBombDropped(const std::string_view& Client_Name) POKEBOT_NOEXCEPT {
		bomber_name = "";
	}

	void Manager::OnBotJoinedCompletely(Bot* const completed_guy) POKEBOT_NOEXCEPT {
		assert(completed_guy->JoinedTeam() == common::Team::T || completed_guy->JoinedTeam() == common::Team::CT);
		const int Team_Index = static_cast<int>(completed_guy->JoinedTeam()) - 1;

		if (auto& troop = troops[Team_Index]; troop.NeedToDevise()) {
			troop.DecideStrategy(&bots, game::game.GetMapFlag());
			troop.Command(&bots);

			// troop.CreatePlatoon([](const BotPair& target) -> bool { return target.second.JoinedPlatoon() == -1; });
		} else {
			if (!completed_guy->JoinedPlatoon().has_value() && troop.GetPlatoonSize() > 0) {
				completed_guy->JoinPlatoon((int)util::Random<int>(0, troop.GetPlatoonSize() - 1));
				troop.Command(completed_guy);
			}
		}
	}

	void Manager::OnMapLoaded() {
		bots.clear();
		bomber_name.clear();
		backpack = nullptr;
		c4_origin = std::nullopt;
		memset(&radio_message, 0, sizeof(radio_message));
		round_started_timer.SetTime(0.0f);

		for (auto& troop : troops) {
			troop.Init();
		}
	}

	node::NodeID Manager::GetGoalNode(const common::Team Target_Team, const PlatoonID Index) const POKEBOT_NOEXCEPT {
		auto& troop = troops[static_cast<int>(Target_Team) - 1];
		if (!Index.has_value()) {
			return troop.GetGoalNode();
		} else {
			return troop.at(*Index).GetGoalNode();
		}
	}

	std::optional<int> Manager::GetTroopTargetedHostage(const common::Team Target_Team, const PlatoonID Index) const noexcept {
		assert(Target_Team == common::Team::CT);
		auto& troop = troops[static_cast<int>(Target_Team) - 1];
		if (!Index.has_value()) {
			return troop.GetTargetHostageIndex();
		} else {
			return troop.at(*Index).GetTargetHostageIndex();
		}
	}

	game::Client* Manager::GetLeader(const common::Team Target_Team, const PlatoonID Index) const POKEBOT_NOEXCEPT {
		auto& troop = troops[static_cast<int>(Target_Team) - 1];
		if (!Index.has_value()) {
			return troop.GetLeader();
		} else {
			return troop.at(*Index).GetLeader();
		}
	}

	bool Manager::IsFollowerPlatoon(const common::Team Target_Team, const PlatoonID Index) const POKEBOT_NOEXCEPT {
		if (!Index.has_value()) {
			return false;
		} else {
			auto& troop = troops[static_cast<int>(Target_Team) - 1];
			return troop.at(*Index).IsStrategyToFollow();
		}
	}
}