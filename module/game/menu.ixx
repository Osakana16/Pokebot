export module pokebot.game: menu;
import :game_manager;

import pokebot.util;

namespace {
	void SplitString(const pokebot::util::fixed_string<256u>& Base_Text, const char Delim, std::vector<pokebot::util::fixed_string<140u>>* result) {
		assert(result != nullptr);
		result->push_back({});

		for (const auto& ch : Base_Text) {
			if (ch == Delim) {
				result->back() += '\0';
				result->push_back({});
			}
			result->back() += ch;
		}
	}
}

export namespace pokebot::game {
	struct CustomRadio final {
		util::fixed_string<60u> message;
		util::fixed_string<60u> sound_path;
	};
	class CommandMenu {
		int slot = 0;
		util::fixed_string<256u> menu_message{};

		game::Array<CustomRadio, 9> item;
	public:
		CommandMenu(util::fixed_string<60u> header, std::initializer_list<CustomRadio> messages) {
			assert(messages.size() > 0 && messages.size() <= 9);

			menu_message = header + "\n\n";
			for (int i = 0; i < messages.size(); i++) {
				auto* message = messages.begin() + i;
				menu_message += std::format("{}. \"{}\"\n", i + 1, message->message.c_str()).c_str();
				item[i] = *message;
				slot |= 1 << i;
			}
			menu_message += "\n0. Exit";
			slot |= 1 << 9;
		}

		void Select(const edict_t* entity, const int Key);
		const util::fixed_string<256u>& Menu_Message = menu_message;
		const int& Slot = slot;
	};

	class Menu final : private game::Singleton<Menu> {
		// de_ map radio menu command.
		CommandMenu demolition_strategy_command_menu{
			"Pokebot Mission Command",
			{
				// - Object unique command -
				CustomRadio{ "Focus efforts on the bombsite closest to CT spawn.", "radio/com_followcom" },		// What bombsite we should defend.
				// The bots cannot recognize bombsites as "A" or "B", so the commander specifes the bombsite by distance.
				CustomRadio{ "Focus efforts on the bombsite furthest from CT spawn.", "radio/com_followcom" },
				CustomRadio{ "Protect bomber.", "radio/com_followcom" },											// Make bots cover the bomber.
				CustomRadio{ "Hold out until this round times out.", "radio/com_followcom" },						// Bots never accomplish the mission in the round. Camp.
				// - Common command -
				CustomRadio{ "Team, infiltrate and eliminate.", "radio/elim" },									// Make bots eliminate only.

			}
		};

		CommandMenu hostage_strategy_command_menu{
			"Pokebot Mission Command",
			{
				CustomRadio{ "Take the rescue zones.", "radio/com_followcom" },	// Camp around the rescue zone.
				CustomRadio{ "Team, infiltrate and eliminate.", "radio/elim" },	// Make bots eliminate only.
			}
		};

		CommandMenu assassination_strategy_command_menu{
			"Pokebot Mission Command",
			{
				CustomRadio{ "Cover the V.I.P.", "radio/com_followcom" },			// Make bots cover the VIP. 
				CustomRadio{ "Take the safety zones.", "radio/com_followcom" },	// Camp around the safety zone.
				CustomRadio{ "Team, infiltrate and eliminate.", "radio/elim" },	// Make bots eliminate only.
			}
		};

		CommandMenu escape_strategy_command_menu{
			"Pokebot Mission Command",
			{
				CustomRadio{ "Take the escape zones.", "radio/com_followcom" },	// Camp around the escape zone.
				CustomRadio{ "Team, infiltrate and eliminate.", "radio/elim" },	// Make bots eliminate only.
			}
		};

		CommandMenu buy_strategy_command_menu{
			"Pokebot Buy Strategy Command",
			{
				CustomRadio{ "Save money.", "radio/com_followcom" },	//
				CustomRadio{ "Buy anything.", "radio/com_followcom" },	//
				CustomRadio{ "Definitely buy defusekit.", "radio/com_followcom" },	//
			}
		};

		CommandMenu strategy_command_menu{
			"Pokebot Strategy Command",
			{
				CustomRadio{ "All sticks together.", "radio/com_followcom" },
				CustomRadio{ "Split up some squads.", "radio/com_followcom" }
			}
		};

		/*
			This command menu is used to control commander's platoon.
		*/
		CommandMenu platoon_command_menu{
			"Pokebot Platoon Command",
			{
				CustomRadio{"Team, let's get out of here!", "radio/getout"},
				CustomRadio{ "Push the T spawn.", "radio/com_followcom" },
				CustomRadio{ "Push the CT spawn.", "radio/com_followcom" }
			}
		};

		// The commands in this variable to reply 'Reporting in, team.'.
		CommandMenu reply_command_menu{
			"Pokebot Reporting Command",
			{
				CustomRadio{
				// This tells the commander that all the bot's platoon members are down.
				"My teammate down.", "radio/matedown"
			},
			CustomRadio{
				// This tells the commander a hostage is down.
				"Hostage down.", "radio/hosdown"
			},
			CustomRadio{
				// This tells the commander the bot is hit.
				// He/She does not need backup, but is not in perfect condition.
				"I'm hit!", "radio/ct_imhit"
			},
			CustomRadio{
				// This tells the commander the bot is hit.
				// His/Her health is low; he/she needs backup.
				"I'm hit, need assistance!", "radio/hitassist"
			}
		}
		};

		CommandMenu* current_menu{};

		void ShowMenu(const edict_t* entity) {
			std::vector<util::fixed_string<140u>> messages{};
			SplitString(current_menu->Menu_Message, '\n', &messages);

			for (int i = 0; i < messages.size(); i++) {
				MESSAGE_BEGIN(MSG_ONE, GET_USER_MSG_ID(PLID, "ShowMenu", NULL), NULL, const_cast<edict_t*>(entity));
				WRITE_SHORT(current_menu->Slot);
				WRITE_CHAR(-1);
				WRITE_BYTE(i != (messages.size() - 1));
				for (const char ch : messages[i]) {
					WRITE_CHAR(ch);
				}
				MESSAGE_END();
			}
		}
	public:
		static Menu& Instance() {
			static Menu menu{};
			return menu;
		}

		enum class SpecifiedCommand {
			Strategy,
			ExRadio,
			Buy_Strategy,
			Platoon_Radio
		};

		/**
		* @brief Close the menu.
		* @param The client that is seeing the menu.
		*/
		void Close(const edict_t* entity) {
			// game.IssueCommand(STRING(entity->v.netname), "menuselect 0");
			MESSAGE_BEGIN(MSG_ONE, GET_USER_MSG_ID(PLID, "ShowMenu", NULL), NULL, const_cast<edict_t*>(entity));
			WRITE_SHORT(0);
			WRITE_CHAR(0);
			WRITE_BYTE(0);
			WRITE_STRING("");
			MESSAGE_END();
			current_menu = nullptr;
		}

		/**
		* @brief Show a specified menu.
		* @param client The client which shows the menu
		* @param specified_command The menu to show.
		* @note This function should be called when pk_menu is executed.
		*/
		void OnCommandRun(const edict_t* entity, SpecifiedCommand specfied_command) {
			Close(entity);
			switch (specfied_command) {
				case SpecifiedCommand::Strategy:
					current_menu = &strategy_command_menu;
					break;
				case SpecifiedCommand::ExRadio:
					current_menu = &demolition_strategy_command_menu;
					break;
				case SpecifiedCommand::Buy_Strategy:
					current_menu = &buy_strategy_command_menu;
					break;
				case SpecifiedCommand::Platoon_Radio:
					current_menu = &platoon_command_menu;
					break;
				default:
					assert(false);
					break;
			}
			ShowMenu(entity);
		}

		/**
		* @brief Execute the process of a menu item.
		* @param client The client which presses an menu item.
		* @param Key The key that is pressed.
		*/
		void OnPress(const edict_t* entity, const int Key) {
			if (current_menu == nullptr) {
				// The other menu(such as 'Team Select') is showing, so do not process.
				return;
			}

			if (Key != 10) {
				current_menu->Select(entity, Key);
			}
			Close(entity);
		}


		void Send(const edict_t* entity, const CustomRadio& Custom_Radio) {
			MESSAGE_BEGIN(MSG_ALL, GET_USER_MSG_ID(PLID, "TextMsg", NULL), nullptr, (edict_t*)nullptr);
			WRITE_BYTE(5);
			WRITE_STRING("1");
			WRITE_STRING("#Game_radio");
			WRITE_STRING(STRING(entity->v.netname));
			WRITE_STRING(Custom_Radio.message.c_str());
			MESSAGE_END();

			meta_engfuncs.pfnMessageBegin(MSG_ALL, GET_USER_MSG_ID(PLID, "TextMsg", NULL), nullptr, (edict_t*)nullptr);
			meta_engfuncs.pfnWriteByte(5);
			meta_engfuncs.pfnWriteString("1");
			meta_engfuncs.pfnWriteString("#Game_radio");
			meta_engfuncs.pfnWriteString(STRING(entity->v.netname));
			meta_engfuncs.pfnWriteString(Custom_Radio.message.c_str());
			meta_engfuncs.pfnMessageEnd();

			CLIENT_COMMAND(const_cast<edict_t*>(entity), std::format("speak \"{}\"\n", Custom_Radio.sound_path.c_str()).c_str());
		}
	};


	void CommandMenu::Select(const edict_t* entity, const int Key) {
		Menu::Instance().Send(entity, item[Key - 1]);
	}
}