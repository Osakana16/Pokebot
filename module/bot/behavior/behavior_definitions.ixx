export module pokebot.bot.behavior: behavior_definitions;
import :action;
import :selector;
import :sequence;
import :condition;
import :behavior_declaration;

import std;
import pokebot.bot;
import pokebot.game;
import pokebot.game.util;
import pokebot.game.player;
import pokebot.terrain.graph.graph_base;
import pokebot.terrain.graph.node;
import pokebot.terrain.goal;
import pokebot.util;


namespace pokebot::bot::behavior {
	namespace demolition {
		std::shared_ptr<Sequence> plant = Sequence::Create("demolition::plant");
		std::shared_ptr<Sequence> defuse = Sequence::Create("demolition::defuse");
	}

	// - Rescue Behaviors -
	namespace rescue {
		std::shared_ptr<Sequence> head_to_hostage = Sequence::Create("head_to_hostage");
		auto get_closer_to_hostage = Selector::Create("use_to_rescue");
		auto use_to_rescue = Sequence::Create("use_to_rescue");
	}

	// - ASsasination Behaviors -
	namespace assassination {
		std::shared_ptr<Sequence> ct_cover = Sequence::Create("assassination::ct_cover");
		std::shared_ptr<Sequence> ct_take_point = Sequence::Create("assassination::ct_take_point");
		std::shared_ptr<Sequence> ct_vip_escape = Sequence::Create("assassination::ct_vip_escape");
	}
	
	// - EScape Behaviors -
	namespace escape {
		std::shared_ptr<Sequence> t_get_primary = Sequence::Create("escape::t_get_primary");
		std::shared_ptr<Sequence> t_take_point = Sequence::Create("escape::t_take_point");
	}

	std::shared_ptr<Sequence> t_ordinary = Sequence::Create("elimination::t_ordinary");
	std::shared_ptr<Sequence> ct_ordinary = Sequence::Create("elimination::ct_ordinary");


	
	namespace fight {
		std::shared_ptr<Sequence> try_to_lose_sight = Sequence::Create("fight::try_to_lose_sight");		// Try to lose the sight from the enemy

		std::shared_ptr<Selector> pick_best_weapon = Selector::Create("fight::pick_best_weapon");

		std::shared_ptr<Selector> while_losing_enemy = Selector::Create("fight::while_losing_enemy");
		std::shared_ptr<Sequence> try_to_find = Sequence::Create("fight::try_to_find");

		std::shared_ptr<Sequence> flee = Sequence::Create("fight::flee");

		std::shared_ptr<Selector> decide_firing = Selector::Create("fight::decide_firing");

		std::shared_ptr<Selector> one_tap_fire = Selector::Create("fight::one_tap_fire");
		std::shared_ptr<Selector> full_burst_fire = Selector::Create("fight::full_burst_fire");

	}
}