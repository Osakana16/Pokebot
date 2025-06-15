#include "plugin.hpp"
#include "navmesh/navigation_map.h"

#include "bot/manager.hpp"

#include "game/menu.hpp"

#include <iostream>
#include <functional>

import pokebot.util.random;
import pokebot.game.util;

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

        static void pk_add_team_specified(const game::Team Default_Team) {
            if (!node::czworld.IsNavFileLoaded()) {
                SERVER_PRINT("[POKEBOT] Error: Cannot add bots because the .nav file is not loaded. Please generate it in CS:CZ.\n");
                return;
            }

            std::vector<std::string_view> args{};
            GetArgs(&args);

            std::string_view name = "FirstBot";
            pokebot::game::Team team = Default_Team;
            pokebot::game::Model model = (pokebot::game::Model)(int)pokebot::util::Random<int>(1, 4);
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
            if (!node::czworld.IsNavFileLoaded()) {
                SERVER_PRINT("[POKEBOT] Error: Cannot add bots because the .nav file does not exist. Please generate it in CS:CZ.\n");
                return;
            }

            std::vector<std::string_view> args{};
            GetArgs(&args);

            std::string_view name = "FirstBot";
            pokebot::game::Team team = (pokebot::game::Team)(int)pokebot::util::Random<int>(1, 2);
            pokebot::game::Model model = (pokebot::game::Model)(int)pokebot::util::Random<int>(1, 4);
            bot::Difficult difficult = bot::Difficult::Normal;
            if (args.size() >= 1) {
                assert(args[0].size() <= 64u);
                name = args[0];
            }

            if (args.size() >= 2) {
                if (args[1] == "1" || args[1] == "T" || args[1] == "t") {
                    team = pokebot::game::Team::T;
                } else if (args[1] == "2" || args[1] == "CT" || args[1] == "ct") {
                    team = pokebot::game::Team::CT;
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
            pk_add_team_specified(game::Team::CT);
        }

        void pk_add_t() {
            pk_add_team_specified(game::Team::T);
        }
    }

    void Pokebot::RegisterCommand() POKEBOT_NOEXCEPT {
        REG_SVR_COMMAND("pk_distance_bombsite", [] {
            edict_t* bombsites[2]{};

            edict_t* edict{};
            while ((edict = game::FindEntityByClassname(edict, "info_bomb_target")) != nullptr) {
                if (bombsites[0] == nullptr) {
                    bombsites[0] = edict;
                } else {
                    bombsites[1] = edict;
                }
                SERVER_PRINT(std::format("[POKEBOT]info_bomb_target: {}\n", game::Distance(game::game.host.Origin(), game::VecBModelOrigin(edict))).c_str());
            }

            while ((edict = game::FindEntityByClassname(edict, "func_bomb_target")) != nullptr) {
                if (bombsites[0] == nullptr) {
                    bombsites[0] = edict;
                } else {
                    bombsites[1] = edict;
                }
                SERVER_PRINT(std::format("[POKEBOT]func_bomb_target: {}\n", game::Distance(game::game.host.Origin(), game::VecBModelOrigin(edict))).c_str());
            }

            if (bombsites[1] != nullptr && bombsites[0] != nullptr) {
                SERVER_PRINT(std::format("[POKEBOT]distance: {}\n", game::Distance(game::VecBModelOrigin(bombsites[0]), game::VecBModelOrigin(bombsites[1]))).c_str());
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

        REG_SVR_COMMAND("pk_navload", [] {
            pokebot::node::czworld.OnMapLoaded();
        });

        REG_SVR_COMMAND("pk_menu", [] {
            std::vector<std::string_view> args{};
            GetArgs(&args);

            if (args.empty())
                return;

            if (args[0] == "1") {
                pokebot::game::Menu::Instance().OnCommandRun(pokebot::game::game.host.AsEdict(), pokebot::game::Menu::SpecifiedCommand::ExRadio);
            } else if (args[0] == "2") {
                pokebot::game::Menu::Instance().OnCommandRun(pokebot::game::game.host.AsEdict(), pokebot::game::Menu::SpecifiedCommand::Buy_Strategy);
            } else if (args[0] == "3") {
                pokebot::game::Menu::Instance().OnCommandRun(pokebot::game::game.host.AsEdict(), pokebot::game::Menu::SpecifiedCommand::Strategy);
            } else if (args[0] == "4") {
                pokebot::game::Menu::Instance().OnCommandRun(pokebot::game::game.host.AsEdict(), pokebot::game::Menu::SpecifiedCommand::Platoon_Radio);
            }
        });

        REG_SVR_COMMAND("pk_what_my_node", [] {
            if (auto area = pokebot::node::czworld.GetNearest(game::game.host.Origin()); area != nullptr) {
                SERVER_PRINT(std::format("My ID:{}\n", area->m_id).c_str());
            } else {
                SERVER_PRINT(std::format("My id is not found.\n").c_str());
            }
        });

        REG_SVR_COMMAND("pk_set_arrow", [] {
            const_cast<edict_t*>(game::game.host.AsEdict())->v.renderfx = 1;
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

    void Pokebot::AddBot(const std::string_view& Bot_Name, const game::Team Selected_Team, const game::Model Selected_Model, const bot::Difficult Difficult) POKEBOT_NOEXCEPT {
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
        pokebot::game::game.clients.Disconnect(STRING(disconnected_client->v.netname));
    }


    void Pokebot::OnMapLoaded() {
#if !USE_NAVMESH
        pokebot::node::world.OnMapLoaded();
#else
        pokebot::node::czworld.OnMapLoaded();
#endif
        pokebot::bot::Manager::Instance().OnMapLoaded();
    }
}