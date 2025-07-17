module pokebot.engine;


namespace pokebot::engine {
	const edict_t* EngineInterface::engine_target_edict;
	int EngineInterface::current_message;
	bool EngineInterface::is_bot_command;

	void EngineInterface::InputFakeclientCommand(edict_t* client, util::fixed_string<32u> sentence) {
		is_bot_command = true;
		bot_args.clear();
		char* arg = strtok(sentence.data(), " ");
		bot_args.push_back(arg);
		while ((arg = strtok(nullptr, " ")) != nullptr) {
			bot_args.push_back(arg);
		}
		MDLL_ClientCommand(const_cast<edict_t*>(client));
		bot_args.clear();
		is_bot_command = false;
	}

	void EngineInterface::OnVGUIMenuShown() {}

	void EngineInterface::OnShowMenu() {
		if (args.size() >= 4) {
			observables.show_menu_observable.NotifyObservers(std::make_tuple(engine_target_edict, std::get<TextCache>(args[3]).c_str()));
		}
	}

	void EngineInterface::OnWeaponListCalled() {

	}

	void EngineInterface::OnTeamInfoCalled() {

	}

	void EngineInterface::OnMoneyChanged() {
		if (args.size() >= 1 && std::holds_alternative<int>(args[0])) {

		}
	}

	void EngineInterface::OnPlayerAmmoPickup() {

	}

	void EngineInterface::OnPlayerDamageTaken() {
		// check the minimum states
		if (args.size() < 3 || !std::holds_alternative<int>(args[0]) || !std::holds_alternative<int>(args[1]) || !std::holds_alternative<int>(args[2])) {
			return;
		}

		using namespace pokebot;

		const int Health = std::get<int>(args[1]);
		const int Armor = std::get<int>(args[0]);
		const int Bit = std::get<int>(args[2]);

		observables.player_damage_taken_observable.NotifyObservers(std::make_tuple(engine_target_edict, engine_target_edict->v.dmg_inflictor, Health, Armor, Bit));
	}

	void EngineInterface::OnStatusIconShown() {
		if (args.size() < 2 || !std::holds_alternative<int>(args[0]) || !std::holds_alternative<TextCache>(args[1])) {
			return;
		}

		using namespace pokebot::game;
		static const std::unordered_map<TextCache, StatusIcon, TextCache::Hash> Icon_Cache{
			{ "c4", StatusIcon::C4 },
			{ "buyzone", StatusIcon::Buy_Zone },
			{ "rescue",  StatusIcon::Rescue_Zone },
			{ "vipsafety", StatusIcon::Vip_Safety },
		};

		if (auto it = Icon_Cache.find(std::get<TextCache>(args[1]).data()); it != Icon_Cache.end()) {
			using namespace pokebot;
			observables.status_icon_observable.NotifyObservers(std::make_tuple(engine_target_edict, it->second));
		}
	}

	void EngineInterface::OnTextMessageSent() {
		auto nothingToDo = [] {};
		auto failIfBotDoes = [] { assert(!is_bot); };
		auto ct_win = [] { observables.ct_win_observable.NotifyObservers(); observables.t_lose_observable.NotifyObservers(); };
		auto t_win = [] { observables.t_win_observable.NotifyObservers(); observables.ct_lose_observable.NotifyObservers(); };

		const std::unordered_map<TextCache, std::function<void()>, TextCache::Hash> Text_Message{
			{ "#Game_connected", nothingToDo },
			{ "#Game_disconnected", nothingToDo },
			{ "#Game_scoring", nothingToDo },
			{ "#Game_join_terrorist", [] { observables.join_t_observable.NotifyObservers(); } },
			{ "#Game_join_ct", [] { observables.join_ct_observable.NotifyObservers(); }},
			{ "#Cstrike_Chat_All", nothingToDo },
			{ "#Cstrike_Chat_CT", nothingToDo },
			{ "#Cstrike_Chat_T", nothingToDo },
			{ "#CTs_Win", ct_win },
			{ "#Terrorists_Win", t_win },
			{ "#Round_Draw", nothingToDo },
			{ "#All_Hostages_Rescued", ct_win },
			{ "#Target_Saved", ct_win },
			{ "#Bomb_Defused", ct_win },
			{ "#Bomb_Planted", [] { observables.bomb_planted_observable.NotifyObservers(); }},
			{ "#Hostages_Not_Rescued", t_win,},
			{ "#VIP_Not_Escaped", t_win },
			{ "#Terrorist_Escaped", t_win },
			{ "#VIP_Assassinated", t_win },
			{ "#Terrorists_Escaped", t_win },
			{ "#Escaping_Terrorists_Neutralized", t_win },
			{ "#Terrorists_Not_Escaped", ct_win },
			{ "#VIP_Escaped", ct_win },
			{ "#CTs_PreventEscape", ct_win },
			{ "#Target_Bombed", t_win },
			{ "#Game_Commencing", [] { observables.game_commercing_observable.NotifyObservers(); }},
			{ "#Game_will_restart_in", nothingToDo },
			{ "#Switch_To_BurstFire", nothingToDo },
			{ "#Switch_To_SemiAuto", nothingToDo },
			{ "#Switch_To_FullAuto", nothingToDo },
			{ "#Game_radio", nothingToDo },
			{ "#Killed_Teammate", nothingToDo },
			{ "#C4_Plant_Must_Be_On_Ground", nothingToDo },
			{ "#Injured_Hostage", nothingToDo },
			{ "#Auto_Team_Balance_Next_Round", nothingToDo },
			{ "#Killed_Hostage", nothingToDo },
			{ "#Too_Many_Terrorists", nothingToDo },
			{ "#Too_Many_CTs", nothingToDo },
			{ "#Weapon_Not_Available", nothingToDo },
			{ "#Game_bomb_pickup", [] { observables.bomb_pickedup_observable.NotifyObservers(); }  },
			{ "#Got_bomb", [] { observables.bomb_pickedup_observable.NotifyObservers(); }},
			{ "#Game_bomb_drop", [] { observables.bomb_dropped_observable.NotifyObservers(); }},
			// TODO: These below should set failIfBotDoes
			{ "#Not_Enough_Money", nothingToDo },
			{ "#Cstrike_Already_Own_Weapon", nothingToDo },
			{ "#Alias_Not_Avail", nothingToDo },
			{ "#Weapon_Cannot_Be_Dropped", nothingToDo },
			{ "#C4_Plant_At_Bomb_Spot", nothingToDo },
			{ "#VIP_cant_buy", nothingToDo },
			{ "#Cannot_Switch_From_VIP", nothingToDo },
			{ "#Only_1_Team_Change", nothingToDo },
			{ "#Cannot_Buy_This", nothingToDo },
			// - Spec Mode(Always nothing to do) -
			{ "#Spec_Mode1", nothingToDo },
			{ "#Spec_Mode2", nothingToDo },
			{ "#Spec_Mode3", nothingToDo },
			{ "#Spec_Mode4", nothingToDo },
			{ "#Spec_Mode5", nothingToDo },
			{ "#Spec_Mode6", nothingToDo },
			{ "#Spec_NoTarget", nothingToDo },
			{ "#Already_Have_One", nothingToDo },
			{ "#Defusing_Bomb_With_Defuse_Kit", nothingToDo },
			{ "#Defusing_Bomb_Without_Defuse_Kit", nothingToDo },
			{ "#Cant_buy", nothingToDo },
			{ "#Game_unknown_command", nothingToDo },
			{ "#Name_change_at_respawn", nothingToDo },
			{ "#Command_Not_Available", nothingToDo }
		};


		using namespace pokebot;
		if (args.size() >= 5 && std::holds_alternative<TextCache>(args[2])) {
			auto it = Text_Message.find(std::get<TextCache>(args[2]).data());
			std::string_view sender = std::get<TextCache>(args[3]).data();
			std::string_view radio = std::get<TextCache>(args[4]).data();

			if (it == Text_Message.find("#Game_radio")) {
				// bot_manager->OnRadioRecieved(sender, radio);
			}
		} else if (args.size() >= 3 && std::holds_alternative<TextCache>(args[2])) {
			Text_Message.at(std::get<TextCache>(args[1]).data())();
		} else if (args.size() >= 2 && std::holds_alternative<TextCache>(args[1])) {
			Text_Message.at(std::get<TextCache>(args[1]).data())();
		}
	}

	void EngineInterface::OnScreenFade() {

	}

	void EngineInterface::OnHLTVStart() {
		// New round message in CS1.6.
		if (args.size() < 2 || !std::holds_alternative<int>(args[0]) || !std::holds_alternative<int>(args[1])) {
			return;
		}

		if (std::get<int>(args[0]) == 0 && std::get<int>(args[1]) == 0) {
			observables.new_round_observable.NotifyObservers();
		}
	}
}