#include "plugin.hpp"
#include "navmesh/navigation_map.h"

#include "bot/manager.hpp"
#include <iostream>
#include <functional>

namespace pokebot::plugin {
    namespace {
        void GetArgs(std::vector<std::string_view>* args) {
            for (int i = 1; ; i++) {
                const char* const arg = CMD_ARGV(i);
                if (arg == nullptr || strlen(arg) <= 0) {
                    break;
                }
                args->push_back(arg);
            }
        }

        static void pk_add_team_specified(const common::Team Default_Team) {
            std::vector<std::string_view> args{};
            GetArgs(&args);

            std::string_view name = "FirstBot";
            pokebot::common::Team team = Default_Team;
            pokebot::common::Model model = (pokebot::common::Model)(int)pokebot::common::Random<int>(1, 4);
            bot::Difficult difficult = bot::Difficult::Normal;
            if (args.size() >= 1) {
                const size_t size = args[0].size();
                name = args[0].substr(0, 32u).data();
            }

            if (args.size() >= 2) {
                model = static_cast<decltype(model)>(std::strtol(args[1].data(), nullptr, 0) % 4);
            }

            if (args.size() >= 3) {
                difficult = static_cast<decltype(difficult)>(std::strtol(args[2].data(), nullptr, 0) % 3);
            }
            pokebot::plugin::pokebot_plugin.AddBot(name.data(), team, model, difficult);
        }

        void pk_add() {
            std::vector<std::string_view> args{};
            GetArgs(&args);

            std::string_view name = "FirstBot";
            pokebot::common::Team team = (pokebot::common::Team)(int)pokebot::common::Random<int>(1, 2);
            pokebot::common::Model model = (pokebot::common::Model)(int)pokebot::common::Random<int>(1, 4);
            bot::Difficult difficult = bot::Difficult::Normal;
            if (args.size() >= 1) {
                assert(args[0].size() <= 64u);
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
                model = static_cast<decltype(model)>(std::strtol(args[2].data(), nullptr, 0) % 4);
            }

            if (args.size() >= 4) {
                difficult = static_cast<decltype(difficult)>(std::strtol(args[3].data(), nullptr, 0) % 4);
            }
            pokebot::plugin::pokebot_plugin.AddBot(name.data(), team, model, difficult);
        }

        void pk_add_ct() {
            pk_add_team_specified(common::Team::CT);
        }

        void pk_add_t() {
            pk_add_team_specified(common::Team::T);
        }
    }

    void Pokebot::RegisterCommand() POKEBOT_NOEXCEPT {
        REG_SVR_COMMAND("pk_distance_bombsite", [] {
            edict_t* bombsites[2]{};

            edict_t* edict{};
            while ((edict = common::FindEntityByClassname(edict, "info_bomb_target")) != nullptr) {
                if (bombsites[0] == nullptr) {
                    bombsites[0] = edict;
                } else {
                    bombsites[1] = edict;
                }
                SERVER_PRINT(std::format("[POKEBOT]info_bomb_target: {}\n", common::Distance(game::game.host.Origin(), common::VecBModelOrigin(edict))).c_str());
            }

            while ((edict = common::FindEntityByClassname(edict, "func_bomb_target")) != nullptr) {
                if (bombsites[0] == nullptr) {
                    bombsites[0] = edict;
                } else {
                    bombsites[1] = edict;
                }
                SERVER_PRINT(std::format("[POKEBOT]func_bomb_target: {}\n", common::Distance(game::game.host.Origin(), common::VecBModelOrigin(edict))).c_str());
            }

            if (bombsites[1] != nullptr && bombsites[0] != nullptr) {
                SERVER_PRINT(std::format("[POKEBOT]distance: {}\n", common::Distance(common::VecBModelOrigin(bombsites[0]), common::VecBModelOrigin(bombsites[1]))).c_str());
            }
        });

        REG_SVR_COMMAND("pk_add", pk_add);
        REG_SVR_COMMAND("pk_add_ct", pk_add_ct);
        REG_SVR_COMMAND("pk_add_t", pk_add_t);

        REG_SVR_COMMAND("pk_auto_waypoint", [] {
            pokebot::game::is_enabled_auto_waypoint = !pokebot::game::is_enabled_auto_waypoint;
        });

        REG_SVR_COMMAND("pk_draw_waypoint", [] {
            draw_node = !draw_node;
        });

        REG_SVR_COMMAND("pk_kill", [] {
            std::vector<std::string_view> args{};
            GetArgs(&args);
            using namespace pokebot;
            game::game.Kill(args[0].data());
        });

        REG_SVR_COMMAND("pk_kill_t",[] {

        });

        REG_SVR_COMMAND("pk_kill_ct", [] {

        });

        REG_SVR_COMMAND("pk_navload", [] {
                pokebot::node::czworld.OnMapLoaded();
        });

        REG_SVR_COMMAND("pk_what_my_node", [] {
            if (auto area = pokebot::node::czworld.GetNearest(game::game.host.Origin()); area != nullptr) {
                SERVER_PRINT(std::format("My ID:{}\n", area->m_id).c_str());
            } else {
                SERVER_PRINT(std::format("My id is not found.\n").c_str());
            }
        });
    }

    void Pokebot::OnUpdate() POKEBOT_NOEXCEPT {
        pokebot::game::game.PreUpdate();
        pokebot::bot::Manager::Instance().Update();
        pokebot::game::game.PostUpdate();
        
        if (draw_node) {
#if !USE_NAVMESH
            pokebot::node::world.Draw();
#endif
        }
    }

    void Pokebot::AddBot(const std::string_view& Bot_Name, const common::Team Selected_Team, const common::Model Selected_Model, const bot::Difficult Difficult) POKEBOT_NOEXCEPT {
        pokebot::bot::Manager::Instance().Insert(Bot_Name.data(), Selected_Team, Selected_Model, Difficult);
    }
    
    void Pokebot::OnEntitySpawned() {
        if (spawned_entity->v.rendermode == kRenderTransTexture) {
            spawned_entity->v.flags &= ~FL_WORLDBRUSH; // clear the FL_WORLDBRUSH flag out of transparent ents
        }
        if (std::string_view classname = STRING(spawned_entity->v.classname); classname == "worldspawn") {
            pWorldEntity = spawned_entity;
            beam_sprite = PRECACHE_MODEL("sprites/laserbeam.spr");
        }        
    }

    void Pokebot::OnClientConnect() {

    }

    void Pokebot::OnClientDisconnect(const edict_t* const disconnected_client) {
        pokebot::bot::Manager::Instance().Remove(STRING(disconnected_client->v.netname));
    }
}