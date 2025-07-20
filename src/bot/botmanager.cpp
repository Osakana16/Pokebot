module pokebot.bot.manager;

import pokebot.bot;
import pokebot.game;
import pokebot.util.random;
import pokebot.game.util;
import pokebot.bot.squad;
import pokebot.terrain.graph.graph_base;
import pokebot.common.event_handler;


namespace pokebot::bot {
	namespace {
		bool IsTerrorist(const BotPair& target) noexcept { return target.second.JoinedTeam() == game::Team::T; }
		bool IsCounterTerrorist(const BotPair& target) noexcept { return target.second.JoinedTeam() == game::Team::CT; }
	}

	Manager::Manager(game::Game& game_,
					 node::Graph& graph_, 
					 game::client::ClientManager& clients_,
					 plugin::Observables* plugin_observables,
					 engine::Observables* engine_observables) : game(game_), graph(graph_), clients(clients_)
	{
		auto newround_callback = [&]() { OnNewRoundPreparation(); };
		plugin_observables->frame_update_observable.AddObserver(std::make_shared<common::NormalObserver<void>>([&]() {
			for (auto& bot : bots) {
				update_observable.NotifyObservers(bot.first);
			}
		}));

		plugin_observables->client_disconnection_observable.AddObserver(std::make_shared<common::NormalObserver<plugin::event::ClientInformation>>([&](const plugin::event::ClientInformation& client_information) {
			Remove(STRING(client_information.entity->v.netname));
		}));
		

		engine_observables->new_round_observable.AddObserver(std::make_shared<common::NormalObserver<void>>(newround_callback));
		engine_observables->show_menu_observable.AddObserver(std::make_shared<common::NormalObserver<std::tuple<const edict_t* const, engine::TextCache>>>([&](const std::tuple<const edict_t* const, engine::TextCache>& args) {
			auto player_name = STRING(std::get<0>(args)->v.netname);
			if (auto bot = Get(player_name); bot != nullptr) {
				static const std::unordered_map<engine::TextCache, pokebot::bot::Message, engine::TextCache::Hash> Menu_Cache{
					{ "#Team_Select", pokebot::bot::Message::Team_Select },
					{ "#Team_Select_Spect", pokebot::bot::Message::Team_Select },
					{ "#IG_Team_Select", pokebot::bot::Message::Team_Select },
					{ "#IG_Team_Select_Spect", pokebot::bot::Message::Team_Select },
					{ "#IG_VIP_Team_Select", pokebot::bot::Message::Team_Select },
					{ "#IG_VIP_Team_Select_Spect", pokebot::bot::Message::Team_Select },
					{ "#Terrorist_Select", pokebot::bot::Message::Model_Select },
					{ "#CT_Select", pokebot::bot::Message::Model_Select }
				};

				if (auto it = Menu_Cache.find(std::get<1>(args).c_str()); it != Menu_Cache.end()) {
					Assign(player_name, it->second);
				}
			}
		}));

		engine_observables->status_icon_observable.AddObserver(std::make_shared<common::NormalObserver<std::tuple<const edict_t* const, game::StatusIcon>>>([&](const std::tuple<const edict_t* const, game::StatusIcon>& args) {
			auto player_name = STRING(std::get<0>(args)->v.netname);
			if (auto bot = Get(player_name); bot != nullptr) {
				clients.OnStatusIconShown(player_name, std::get<1>(args));
			}
		}));
	}

	void Manager::OnNewRoundPreparation() noexcept {
		auto convert_to_set = [&](const game::Team target_team) {
			std::unordered_set<util::PlayerName, util::PlayerName::Hash> members{};
			for (auto& bot : bots | std::views::filter([target_team](const std::pair<pokebot::util::PlayerName, Bot>& pair) { return pair.second.JoinedTeam() == target_team; })) {
				members.insert(bot.first);
			}
			return members;
		};
		
		std::unordered_set<util::PlayerName, util::PlayerName::Hash> terrorists = convert_to_set(game::Team::T);
		std::unordered_set<util::PlayerName, util::PlayerName::Hash> cts = convert_to_set(game::Team::CT);
		terrorist_troops = std::make_unique<pokebot::bot::squad::Troops>(game::Team::T);
		ct_troops = std::make_unique<pokebot::bot::squad::Troops>(game::Team::CT);
		
		terrorist_troops->UpdateMember(terrorists);
		ct_troops->UpdateMember(cts);

		terrorist_troops->Establish(&game, &graph);
		ct_troops->Establish(&game, &graph);

		initialization_stage = InitializationStage::Player_Action_Ready;
		round_started_timer.SetTime(1.0f);

		for (auto& bot : bots) {
			if (bot.second.JoinedTeam() == game::Team::T) 
				bot.second.OnNewRound(terrorist_troops.get());
			else
				bot.second.OnNewRound(ct_troops.get());
		}
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
		const auto Leader_Client = clients.Get(Sender_Name.data());

		radio_message.team = Leader_Client->GetTeam();
		radio_message.sender = Sender_Name.data();
		radio_message.message = Radio_Sentence.data();
	}

	void Manager::Insert(pokebot::util::PlayerName bot_name, const game::Team team, game::client::ClientManager& clients, const game::Model model) POKEBOT_NOEXCEPT {
		if (auto spawn_result = clients.Create(bot_name.c_str()); std::get<bool>(spawn_result)) {
			const bool Is_First_Bot = bots.empty();
			if (Is_First_Bot) {
				terrorist_troops = std::make_unique<pokebot::bot::squad::Troops>(game::Team::T);
				ct_troops = std::make_unique<pokebot::bot::squad::Troops>(game::Team::CT);
			}

			bot_name = std::get<pokebot::util::PlayerName>(spawn_result).c_str();
			auto insert_result = bots.emplace(
				std::piecewise_construct,
				std::forward_as_tuple(bot_name.c_str()),
				std::forward_as_tuple(
					&new_round_observables,
					&update_observable, 
					&radio_sent_observable,
					&bomb_dropped,
					&bomb_pickedup,
					&dead_observable,
					(team == game::Team::T ? terrorist_troops : ct_troops).get(), 
					game, 
					graph,
					clients,
					bot_name.c_str(),
					team,
					model)
			);

			if (Is_First_Bot) {
				auto convert_to_set = [&](const game::Team target_team) {
					std::unordered_set<util::PlayerName, util::PlayerName::Hash> members{};
					for (auto& bot : bots | std::views::filter([target_team](const std::pair<pokebot::util::PlayerName, Bot>& pair) { return pair.second.JoinedTeam() == target_team; })) {
						members.insert(bot.first);
					}
					return members;
				};
				std::unordered_set<util::PlayerName, util::PlayerName::Hash> terrorists = convert_to_set(game::Team::T);
				std::unordered_set<util::PlayerName, util::PlayerName::Hash> cts = convert_to_set(game::Team::CT);

				terrorist_troops->UpdateMember(terrorists);
				ct_troops->UpdateMember(cts);

				terrorist_troops->Establish(&game, &graph);
				ct_troops->Establish(&game, &graph);

			}
			assert(insert_result.second);
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
		if (auto bot = Get(Bot_Name); bot != nullptr) {
			new_round_observables.Remove(Bot_Name.data());
			update_observable.Remove(Bot_Name.data());
			disconnection_observable.Remove(Bot_Name.data());
			radio_sent_observable.Remove(Bot_Name.data());
			bomb_dropped.Remove(Bot_Name.data());
			bomb_pickedup.Remove(Bot_Name.data());
			dead_observable.Remove(Bot_Name.data());

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
	}

	void Manager::OnBombDropped(const std::string_view& Client_Name) POKEBOT_NOEXCEPT {
		bomber_name = "";
	}

	void Manager::OnMapLoaded() {
		bots.clear();
		bomber_name.clear();
		memset(&radio_message, 0, sizeof(radio_message));
		round_started_timer.SetTime(0.0f);
	}
}