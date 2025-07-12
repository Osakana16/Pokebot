module;
#include "navmesh/navigation_map.h"

module pokebot.terrain.graph: cznav_graph;

import pokebot.game.util;
import pokebot.util;
import pokebot.common.event_handler;

namespace pokebot::node {
	CZBotGraph::CZBotGraph(plugin::Observables* plugin_observables, engine::Observables* engine_observables) {
		auto callback = [&](const util::fixed_string<256u>& map_name) {
			if (!navigation_map.Load(std::format("cstrike/maps/{}.nav", map_name.c_str()))) {
				if (!navigation_map.Load(std::format("czero/maps/{}.nav", map_name.c_str()))) {
					SERVER_PRINT("[POKEBOT]Failed to load the nav file.\n");
					is_nav_loaded = false;
					return;
				} else {
					SERVER_PRINT("[POKEBOT]Loaded the nav file from czero.\n");
				}
			} else {
				SERVER_PRINT("[POKEBOT]Loaded the nav file from cstrike.\n");
			}

			is_nav_loaded = true;
			auto addGoal = [this](const GoalKind kind, const char* class_name, Vector(*originFunction)(edict_t*)) {
				edict_t* entity = nullptr;
				while ((entity = game::FindEntityByClassname(entity, class_name)) != nullptr) {
					Vector origin = originFunction(entity);
					auto area = GetNearest(origin);
					if (area != nullptr) {
						goals.insert({ kind, area->m_id });
					}
				}
			};

			auto returnOrigin = [](edict_t* entity) { return entity->v.origin; };
			auto returnModelOrigin = [](edict_t* entity) { return game::VecBModelOrigin(entity); };

			goals.clear();
			addGoal(GoalKind::CT_Spawn, "info_player_start", returnOrigin);				// CT Spawn
			addGoal(GoalKind::Terrorist_Spawn, "info_player_deathmatch", returnOrigin);	// Terrorist Spawn
			addGoal(GoalKind::Rescue_Zone, "func_hostage_rescue", returnModelOrigin);
			addGoal(GoalKind::Rescue_Zone, "info_hostage_rescue", returnModelOrigin);
			addGoal(GoalKind::Bombspot, "func_bomb_target", returnModelOrigin);
			addGoal(GoalKind::Bombspot, "info_bomb_target", returnModelOrigin);
			addGoal(GoalKind::Vip_Safety, "info_vip_start", returnModelOrigin);
			addGoal(GoalKind::Vip_Safety, "func_vip_safetyzone", returnModelOrigin);
			addGoal(GoalKind::Escape_Zone, "func_escapezone", returnModelOrigin);
		};

		plugin_observables->map_loaded_observable.AddObserver(std::make_shared<common::NormalObserver<util::fixed_string<256u>>>(callback));
	}
}