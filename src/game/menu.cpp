#include "game/menu.hpp"

import pokebot.game;

namespace pokebot::game {
	namespace {
		void SplitString(const util::fixed_string<256u>& Base_Text, const char Delim, std::vector<util::fixed_string<140u>>* result) {
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

	CommandMenu::CommandMenu(util::fixed_string<60u> header, std::initializer_list<CustomRadio> messages) {
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

	void CommandMenu::Select(const edict_t* entity, const int Key) {
		Menu::Instance().Send(entity, item[Key - 1]);
	}

	void Menu::Send(const edict_t* entity, const CustomRadio& Custom_Radio) {
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


	void Menu::OnCommandRun(const edict_t* entity, SpecifiedCommand specfied_command) {
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

	void Menu::Close(const edict_t* entity) {
		game::game.IssueCommand(STRING(entity->v.netname), "menuselect 0");
		MESSAGE_BEGIN(MSG_ONE, GET_USER_MSG_ID(PLID, "ShowMenu", NULL), NULL, const_cast<edict_t*>(entity));
		WRITE_SHORT(0);
		WRITE_CHAR(0);
		WRITE_BYTE(0);
		WRITE_STRING("");
		MESSAGE_END();
		current_menu = nullptr;
	}

	void Menu::OnPress(const edict_t* entity, const int Key) {
		if (current_menu == nullptr) {
			// The other menu(such as 'Team Select') is showing, so do not process.
			return;
		}

		if (Key != 10) {
			current_menu->Select(entity, Key);
		}
		Close(entity);
	}

	void Menu::ShowMenu(const edict_t* entity) {
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
}