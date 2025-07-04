module pokebot.plugin.console.command: pk_menu;
import :console_args;

import pokebot.game;

void pk_menu() {
    std::vector<std::string_view> args{};
    get_console_args(&args);

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
}