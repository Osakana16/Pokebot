module pokebot.plugin.console.command: pk_bot;
import :console_args;

void pk_kill() {
    std::vector<std::string_view> args{};
    get_console_args(&args);
    // using namespace pokebot;
    // game::game.Kill(args[0].data());
}

void pk_kill_t() {

}

void pk_kill_ct() {

}