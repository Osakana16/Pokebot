module pokebot.bot: bot_manager;

import pokebot.bot;
import pokebot.game;
import pokebot.util.random;
import pokebot.game.util;
import pokebot.bot.squad;
import pokebot.terrain.graph;
namespace pokebot::bot {
	namespace {
		bool IsTerrorist(const BotPair& target) noexcept { return target.second.JoinedTeam() == game::Team::T; }
		bool IsCounterTerrorist(const BotPair& target) noexcept { return target.second.JoinedTeam() == game::Team::CT; }
	}

	Manager::Manager() {}


	void Manager::OnNewRoundPreparation() noexcept {
		auto convert_to_set = [&](const game::Team target_team) {
			std::unordered_set<util::PlayerName, util::PlayerName::Hash> members{};
			for (auto& bot : bots | std::views::filter([target_team](const std::pair<pokebot::util::PlayerName, Bot>& pair) { return pair.second.JoinedTeam() == target_team; })) {
				members.insert(bot.first);
			}
			return members;
		};

		for (auto& bot : bots) {
			bot.second.OnNewRound();
		}
		c4_origin = std::nullopt;
		
		std::unordered_set<util::PlayerName, util::PlayerName::Hash> terrorists = convert_to_set(game::Team::T);
		std::unordered_set<util::PlayerName, util::PlayerName::Hash> cts = convert_to_set(game::Team::CT);
		terrorist_troops = std::make_unique<pokebot::bot::squad::Troops>(game::Team::T, terrorists);
		ct_troops = std::make_unique<pokebot::bot::squad::Troops>(game::Team::CT, cts);

		terrorist_troops->Establish(&game::game, &node::czworld);
		ct_troops->Establish(&game::game, &node::czworld);

		initialization_stage = InitializationStage::Player_Action_Ready;
		round_started_timer.SetTime(1.0f);
	}

	void Manager::OnNewRoundReady() POKEBOT_NOEXCEPT {
		if (!round_started_timer.IsRunning() && initialization_stage != InitializationStage::Player_Action_Ready) {
			return;
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
			bot->current_weapon = game::weapon::ID::None;
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
	}

	void Manager::Insert(pokebot::util::PlayerName bot_name, const game::Team team, const game::Model model) POKEBOT_NOEXCEPT {
		if (auto spawn_result = game::game.clients.Create(bot_name.c_str()); std::get<bool>(spawn_result)) {
			bot_name = std::get<pokebot::util::PlayerName>(spawn_result).c_str();
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
						while ((dropped_bomb = game::FindEntityByClassname(dropped_bomb, "weaponbox")) != NULL) {
							if (std::string_view(STRING(dropped_bomb->v.model)) == "models/w_backpack.mdl") {
								backpack = dropped_bomb;

								break;
							}
						}
					}
				}

				edict_t* c4{};
				while ((c4 = game::FindEntityByClassname(c4, "grenade")) != nullptr) {
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
	}

	Bot* const Manager::Get(const std::string_view& Bot_Name) noexcept {
		auto bot_iterator = bots.find(Bot_Name.data());
		return (bot_iterator != bots.end() ? &bot_iterator->second : nullptr);
	}

	const Bot* const Manager::Get(const std::string_view& Bot_Name) const noexcept {
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

	void Manager::OnMapLoaded() {
		bots.clear();
		bomber_name.clear();
		backpack = nullptr;
		c4_origin = std::nullopt;
		memset(&radio_message, 0, sizeof(radio_message));
		round_started_timer.SetTime(0.0f);
	}

	node::NodeID Manager::GetGoalNode(const std::string_view& Client_Name) const noexcept {
		switch (const game::Team Target_Team = Get(Client_Name)->JoinedTeam(); Target_Team) {
			case game::Team::T:
				return terrorist_troops->GetPlatoonGoal(Client_Name.data());
			case game::Team::CT:
				return ct_troops->GetPlatoonGoal(Client_Name.data());
		}
		return {};
	}
}