#pragma once
#include "util/fixed_string.hpp"

namespace pokebot::game {
	struct CustomRadio final {
		util::fixed_string<60u> message;
		util::fixed_string<60u> sound_path;
	};

	class CommandMenu {
		int slot = 0;
		util::fixed_string<256u> menu_message{};

		common::Array<CustomRadio, 9> item;
	public:
		CommandMenu(util::fixed_string<60u> header, std::initializer_list<CustomRadio> messages);

		void Select(const edict_t* entity, const int Key);

		const util::fixed_string<256u>& Menu_Message = menu_message;
		const int& Slot = slot;
	};

	class Menu final : private common::Singleton<Menu> {
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

		CommandMenu *current_menu{};

		void ShowMenu(const edict_t*);
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
		void Close(const edict_t* client);

		/**
		* @brief Show a specified menu.
		* @param client The client which shows the menu
		* @param specified_command The menu to show.
		* @note This function should be called when pk_menu is executed.
		*/
		void OnCommandRun(const edict_t* client, SpecifiedCommand specified_command);

		/**
		* @brief Execute the process of a menu item.
		* @param client The client which presses an menu item.
		* @param Key The key that is pressed.
		*/
		void OnPress(const edict_t* client, const int Key);

		
		void Send(const edict_t* entity, const CustomRadio& Custom_Radio);
	};
}