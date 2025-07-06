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
			observables.show_menu_observable.Notifyobservers(std::make_tuple(engine_target_edict, std::get<TextCache>(args[3]).c_str()));
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

		observables.player_damage_taken_observable.Notifyobservers(std::make_tuple(engine_target_edict, engine_target_edict->v.dmg_inflictor, Health, Armor, Bit));
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
			observables.status_icon_observable.Notifyobservers(std::make_tuple(engine_target_edict, it->second));
		}
	}

	void EngineInterface::OnTextMessageSent() {
		auto nothingToDo = [] {};
		auto failIfBotDoes = [] { assert(!is_bot); };

		const std::unordered_map<TextCache, std::function<void()>, TextCache::Hash> Text_Message{
			{ "#Game_connected", nothingToDo },
			{ "#Game_disconnected", nothingToDo },
			{ "#Game_scoring", nothingToDo },
			{ "#Game_join_terrorist", nothingToDo },
			{ "#Game_join_ct", nothingToDo },
			{ "#Cstrike_Chat_All", nothingToDo },
			{ "#Cstrike_Chat_CT", nothingToDo },
			{ "#Cstrike_Chat_T", nothingToDo },
			{ "#CTs_Win", nothingToDo },
			{ "#Terrorists_Win", nothingToDo },
			{ "#Round_Draw", nothingToDo },
			{ "#All_Hostages_Rescued", nothingToDo },
			{ "#Target_Saved", nothingToDo },
			{ "#Bomb_Defused", nothingToDo },
			{ "#Bomb_Planted", nothingToDo },
			{ "#Hostages_Not_Rescued", nothingToDo,},
			{ "#Terrorists_Not_Escaped", nothingToDo },
			{ "#VIP_Not_Escaped", nothingToDo },
			{ "#Escaping_Terrorists_Neutralized", nothingToDo },
			{ "#Terrorist_Escaped", nothingToDo },
			{ "#VIP_Assassinated", nothingToDo },
			{ "#VIP_Escaped", nothingToDo },
			{ "#Terrorists_Escaped", nothingToDo },
			{ "#CTs_PreventEscape", nothingToDo },
			{ "#Target_Bombed", nothingToDo },
			{ "#Game_Commencing", nothingToDo },
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
			{ "#Game_bomb_pickup", [] {}  },
			{ "#Got_bomb", nothingToDo },
			{ "#Game_bomb_drop", [] { /*bot_manager->OnBombDropped(std::get<TextCache>(args[2]).data());*/ }},
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
			observables.new_round_observable.Notifyobservers();
		}
	}
}