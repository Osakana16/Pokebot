#include "plugin.hpp"
#include "navmesh/navigation_map.h"

#include <iostream>
#include <functional>


// ラッパークラスを作成
template <typename R, typename... Args>
struct function_traits;

template <typename R, typename... Args>
struct function_traits<R(Args...)> {
    using function_type = R(*)(Args...);
};

template <typename R, typename... Args>
typename function_traits<R(Args...)>::function_type to_function_pointer(std::function<R(Args...)>& func) {
    return *func.target<typename function_traits<R(Args...)>::function_type>();
}

namespace pokebot::plugin {
    void Pokebot::RegisterCommand() noexcept {
        static auto GetArgs = []() {
            static std::vector<std::string> args{};
            for (int i = 1; ; i++) {
                auto arg = CMD_ARGV(i);
                if (arg == nullptr || strlen(arg) <= 0) {
                    break;
                }
                args.push_back(arg);
            }
            return std::move(args);
        };

        REG_SVR_COMMAND(
            "pk_add",
            [] {
            auto args = GetArgs();

            std::string name = "FirstBot";
            pokebot::common::Team team = (pokebot::common::Team)(int)pokebot::common::Random<int>(1, 2);
            pokebot::common::Model model = (pokebot::common::Model)(int)pokebot::common::Random<int>(1, 4);
            bot::Difficult difficult = bot::Difficult::Normal;
            if (args.size() >= 1) {
                name = args[0];
            }

            if (args.size() >= 2) {
                if (args[1] == "1" || args[1] == "T" || args[1] == "t") {
                    team = pokebot::common::Team::T;
                } else if (args[1] == "2" || args[1] == "CT" || args[1] == "ct") {
                    team = pokebot::common::Team::CT;
                }
            }

            if (args.size() >= 3) {
                model = static_cast<decltype(model)>(std::strtol(args[2].c_str(), nullptr, 0) % 4);
            }

            if (args.size() >= 4) {
                difficult = static_cast<decltype(difficult)>(std::strtol(args[3].c_str(), nullptr, 0) % 4);
            }
            pokebot::plugin::pokebot_plugin.AddBot(name, team, model, difficult);
        }
        );

        REG_SVR_COMMAND(
            "pk_add_ct",
            [] {
            auto args = GetArgs();

            std::string name = "FirstBot";
            pokebot::common::Team team = pokebot::common::Team::CT;
            pokebot::common::Model model = (pokebot::common::Model)(int)pokebot::common::Random<int>(1, 4);
            bot::Difficult difficult = bot::Difficult::Normal;
            if (args.size() >= 1) {
                name = args[0];
            }

            if (args.size() >= 2) {
                model = static_cast<decltype(model)>(std::strtol(args[1].c_str(), nullptr, 0) % 4);
            }

            if (args.size() >= 3) {
                difficult = static_cast<decltype(difficult)>(std::strtol(args[2].c_str(), nullptr, 0) % 3);
            }
            pokebot::plugin::pokebot_plugin.AddBot(name, team, model, difficult);
        }
        );

        REG_SVR_COMMAND(
            "pk_add_t",
            [] {
            auto args = GetArgs();

            std::string name = "FirstBot";
            pokebot::common::Team team = pokebot::common::Team::T;
            pokebot::common::Model model = (pokebot::common::Model)(int)pokebot::common::Random<int>(1, 4);
            bot::Difficult difficult = bot::Difficult::Normal;
            if (args.size() >= 1) {
                name = args[0];
            }

            if (args.size() >= 2) {
                model = static_cast<decltype(model)>(std::strtol(args[1].c_str(), nullptr, 0) % 4);
            }

            if (args.size() >= 3) {
                difficult = static_cast<decltype(difficult)>(std::strtol(args[2].c_str(), nullptr, 0) % 3);
            }
            pokebot::plugin::pokebot_plugin.AddBot(name, team, model, difficult);
        }
        );

        REG_SVR_COMMAND(
            "pk_auto_waypoint",
            [] {
            pokebot::game::is_enabled_auto_waypoint = !pokebot::game::is_enabled_auto_waypoint;
        }
        );

        REG_SVR_COMMAND(
            "pk_draw_waypoint",
            [] {
            draw_node = !draw_node;
        }
        );

        REG_SVR_COMMAND(
            "pk_kill",
            [] {
            auto args = GetArgs();
            using namespace pokebot;
            MDLL_ClientKill(*game::game.clients.Get(args[0]));
        }
        );

        REG_SVR_COMMAND(
            "pk_kill_t",
            [] {

        }
        );

        REG_SVR_COMMAND(
            "pk_kill_ct",
            [] {
        }
        );

        REG_SVR_COMMAND(
            "pk_navload",
            [] {
                pokebot::node::czworld.OnMapLoaded();
            }
        );

        // to_function_pointer(nullptr);

        REG_SVR_COMMAND
        (
            "pk_findpath",
            [] {
                edict_t* entity{};
                Vector origin{};
		        while ((entity = common::FindEntityByClassname(entity, "func_bomb_target")) != nullptr) {
			        origin = common::VecBModelOrigin(entity);
                    break;
		        }

                node::PathWalk<std::uint32_t> routes{};
                pokebot::node::czworld.FindPath(&routes, pokebot::game::game.host.Origin(), origin);
            }
        );
    }

    void Pokebot::OnUpdate() noexcept {
        pokebot::bot::manager.Update();
        pokebot::game::game.Update();
        
        if (draw_node) {
#if !USE_NAVMESH
            pokebot::node::world.Draw();
#endif
        }
    }

    void Pokebot::AddBot(const std::string& Bot_Name, const common::Team Selected_Team, const common::Model Selected_Model, const bot::Difficult Difficult) POKEBOT_DEBUG_NOEXCEPT {
        pokebot::bot::manager.Insert(Bot_Name, Selected_Team, Selected_Model, Difficult);
    }
    
    void Pokebot::OnEntitySpawned() {
        std::string classname = STRING(spawned_entity->v.classname);
        if (spawned_entity->v.rendermode == kRenderTransTexture) {
            spawned_entity->v.flags &= ~FL_WORLDBRUSH; // clear the FL_WORLDBRUSH flag out of transparent ents
        }
        if (classname == "worldspawn") {
            pWorldEntity = spawned_entity;
            beam_sprite = PRECACHE_MODEL("sprites/laserbeam.spr");
        }        
    }

    void Pokebot::OnClientConnect() {

    }

    void Pokebot::OnClientDisconnect(const edict_t* const disconnected_client) {
        pokebot::bot::manager.Remove(STRING(disconnected_client->v.netname));
    }
}