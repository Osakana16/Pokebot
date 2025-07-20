module pokebot.plugin.console.command: pk_bot;
import :console_args;

import pokebot.util.random;
import pokebot.game.util;
import pokebot.api.command_executors;

static void pk_add_team_specified(const pokebot::game::Team Default_Team) {
    if (!pokebot::api::command_executor->IsPlayable()) {
        SERVER_PRINT("[POKEBOT] Error: Cannot add bots because the .nav file is not loaded. Please generate it in CS:CZ.\n");
        return;
    }

    std::vector<std::string_view> args{};
    get_console_args(&args);

    std::string_view name = "FirstBot";
    pokebot::game::Team team = Default_Team;
    pokebot::game::Model model = (pokebot::game::Model)(int)pokebot::util::Random<int>(1, 4);
    if (args.size() >= 1) {
        const size_t size = args[0].size();
        name = args[0].substr(0, 32u).data();
    }

    if (args.size() >= 2) {
        model = static_cast<decltype(model)>(std::strtol(args[1].data(), nullptr, 0) % 4);
    }

    pokebot::api::command_executor->AddBot(name.data(), team, model);
    // pokebot::plugin::Pokebot::AddBot(name.data(), team, model);
}

void pk_add() {
    if (!pokebot::api::command_executor->IsPlayable()) {
        SERVER_PRINT("[POKEBOT] Error: Cannot add bots because the .nav file does not exist. Please generate it in CS:CZ.\n");
        return;
    }

    std::vector<std::string_view> args{};
    get_console_args(&args);

    std::string_view name = "FirstBot";
    pokebot::game::Team team = (pokebot::game::Team)(int)pokebot::util::Random<int>(1, 2);
    pokebot::game::Model model = (pokebot::game::Model)(int)pokebot::util::Random<int>(1, 4);
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
    pokebot::api::command_executor->AddBot(name.data(), team, model);
    // pokebot::plugin::Pokebot::AddBot(name.data(), team, model);
}

void pk_add_ct() {
    pk_add_team_specified(pokebot::game::Team::CT);
}

void pk_add_t() {
    pk_add_team_specified(pokebot::game::Team::T);
}
