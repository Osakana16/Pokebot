module;
#include "goldsrc.hpp"
module pokebot.game.scenario: demolition_manager;
import pokebot.game.util;

namespace pokebot::game::scenario {
	void DemolitionManager::Update() {
		if (!c4_origin.has_value()) {
			if (bomber_name.empty()) {
				// When the bomb is dropped:
				// 
				if (backpack_origin.has_value()) {

				} else {
					edict_t* dropped_bomb{};
					while ((dropped_bomb = game::FindEntityByClassname(dropped_bomb, "weaponbox")) != NULL) {
						if (std::string_view(STRING(dropped_bomb->v.model)) == "models/w_backpack.mdl") {
							*backpack_origin = dropped_bomb->v.origin;
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

			}
		}
	}
}