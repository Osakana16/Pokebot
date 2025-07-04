export module pokebot.bot.state: accomplish;
import :bot_state_element;

import pokebot.bot.behavior;
import pokebot.game;
import pokebot.game.weapon;
import pokebot.game.util;

export namespace pokebot::bot::state {
	template<pokebot::game::Team team>
	class AccomplishState : public BotStateElement {
	public:
		using BotStateElement::BotStateElement;

		void Run() final {
			if constexpr (team == pokebot::game::Team::T) {
				if (Manager::Instance().C4Origin().has_value()) {
					behavior::demolition::t_planted_wary->Evaluate(self);
				} else if (Manager::Instance().BackpackOrigin().has_value()) {
					behavior::demolition::t_pick_up_bomb->Evaluate(self);
				} else {
					if (self->HasWeapon(game::weapon::ID::C4)) {
						behavior::demolition::t_plant->Evaluate(self);
					}
				}
			} else if constexpr (team == pokebot::game::Team::CT) {
				if (Manager::Instance().C4Origin().has_value()) {
					if (game::Distance(self->Origin(), *Manager::Instance().C4Origin()) <= 50.0f) {
						behavior::demolition::ct_defusing->Evaluate(self);
					} else {
						behavior::demolition::ct_planted->Evaluate(self);
					}
				} else {
					behavior::demolition::ct_defend->Evaluate(self);
				}
			}
		}
	};
}